#pragma once
#include "Asio.hpp"
#include "Message.hpp"
#include "utils/make_variant_of_nth.hpp"
#include <boost/asio/buffer.hpp>
#include <boost/asio/completion_condition.hpp>
#include <boost/asio/read.hpp>
#include <boost/endian/conversion.hpp>
#include <boost/fusion/include/for_each.hpp>
#include <boost/fusion/include/out.hpp>
#include <chrono>
#include <iostream>

template <typename AsyncReadStream>
class MessageReader final {
public:
    template <typename Duration = std::chrono::seconds>
    MessageReader (AsyncReadStream& stream,
                   Duration timeout = std::chrono::seconds (3))
        : stream_ (&stream),
          timer_ (stream.get_io_service ()),
          read_timeout_ (
              std::chrono::duration_cast<decltype (read_timeout_)> (timeout)) {
    }

    auto
    error () const noexcept {
        return ec_;
    }

    void
    reset () {
        buffer_.clear ();
        read_size_ = 0;
        if (started_) {
            timer_.cancel ();
            started_ = false;
        }
    }

    std::size_t
    read_chunk (boost::system::error_code ec,
                std::size_t const bytes_transferred) {
        if (ec || (bytes_transferred >= read_size_)) {
            return 0;
        }

        if (!started_ && bytes_transferred) {
            started_ = true;
            timer_.expires_from_now (read_timeout_);
            timer_.async_wait ([this](auto const ec) {
                if (ec == asio::error::operation_aborted) {
                    return;
                } else if (ec) {
                    throw boost::system::system_error (ec, ec.message ());
                }
                stream_->lowest_layer().cancel ();
            });
        }

        auto const bytes_left = read_size_ - bytes_transferred;
        return std::min (
            bytes_left,
            std::size_t (boost::asio::detail::default_max_transfer_size));
    }

    bool
    read_header (MessageHeader& header, asio::yield_context ctx) {
        read_size_ = sizeof (header);

        auto const bytes_read = asio::async_read (
            *stream_,
            asio::mutable_buffers_1 (&header, sizeof (header)),
            [this](auto&&... args) {
                return this->read_chunk (
                    std::forward<decltype (args)> (args)...);
            },
            ctx[ec_]);

        if (bytes_read != sizeof (header)) {
            reset ();
            return false;
        }

        boost::fusion::for_each (header, [](auto& field) {
            boost::endian::little_to_native_inplace (field);
        });

        if (header.ttl) {
            --header.ttl;
        }
        return true;
    }

    bool
    read_body (boost::blank&, asio::yield_context) {
        return true;
    }

    template <typename T>
    std::enable_if_t<std::is_base_of<flatbuffers::NativeTable, T>::value, bool>
    read_body (T& body, asio::yield_context ctx) {
        buffer_.resize (read_size_);

        auto const bytes_read =
            asio::async_read (*stream_,
                              asio::buffer (buffer_.data (), buffer_.size ()),
                              [this](auto&&... args) {
                                  return this->read_chunk (
                                      std::forward<decltype (args)> (args)...);
                              },
                              ctx[ec_]);

        if (bytes_read != read_size_) {
            return false;
        }

        flatbuffers::Verifier fbv (buffer_.data (), buffer_.size ());
        if (!fbv.VerifyBuffer<typename T::TableType> ()) {
            return false;
        }

        auto root =
            flatbuffers::GetRoot<typename T::TableType> (buffer_.data ());
        root->UnPackTo (&body);
        return true;
    }

    bool
    read_body (std::vector<char>& body, asio::yield_context ctx) {
        body.resize (read_size_);

        auto const bytes_read = asio::async_read (
            *stream_,
            asio::buffer (const_cast<char*> (body.data ()), body.size ()),
            [this](auto&&... args) {
                return this->read_chunk (
                    std::forward<decltype (args)> (args)...);
            },
            ctx[ec_]);

        if (bytes_read != read_size_) {
            body.clear ();
            return false;
        }

        return true;
    }

    bool
    read_body (MessageHeader& header, Message& message,
               asio::yield_context ctx) {
        bool success  = false;
        message.type_ = header.type;
        message.ttl_  = header.ttl;
        message.body_ = make_variant_of_nth<Message::Body> (
            header.type, &success, [this, &header, &success, ctx](auto&& body) {
                read_size_ = header.size;
                success    = this->read_body (body, ctx);
                return std::move (body);
            });

        reset ();
        return success;
    }

private:
    AsyncReadStream* stream_;
    asio::steady_timer timer_;
    std::vector<unsigned char> buffer_;
    std::chrono::milliseconds read_timeout_;
    boost::system::error_code ec_;
    std::size_t read_size_ = 0;
    bool started_          = false;
};
