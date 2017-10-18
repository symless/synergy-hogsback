#pragma once
#include "Asio.hpp"
#include "Message.hpp"
#include <boost/asio/buffer.hpp>
#include <boost/asio/write.hpp>
#include <boost/endian/conversion.hpp>
#include <boost/fusion/include/for_each.hpp>

template <typename AsyncWriteStream>
struct MessageWriter final {
public:
    explicit MessageWriter (AsyncWriteStream& stream) : stream_ (&stream) {
    }

    template <typename T>
    std::enable_if_t<std::is_base_of<flatbuffers::NativeTable, T>::value, bool>
    do_write (MessageHeader& header, T const& body, asio::yield_context ctx) {
        fbb_.Clear ();
        fbb_.Finish (T::TableType::Pack (fbb_, &body));
        header.size = fbb_.GetSize ();

        std::array<asio::const_buffer, 2> const buffers = {
            asio::const_buffer (&header, sizeof (header)),
            asio::const_buffer (fbb_.GetBufferPointer (), header.size)};

        boost::fusion::for_each (header, [](auto& field) {
            boost::endian::native_to_little_inplace (field);
        });

        auto const bytes_written =
            asio::async_write (*stream_, buffers, ctx[ec_]);
        return (!ec_ && (bytes_written == (sizeof (header) + header.size)));
    }

    bool
    do_write (MessageHeader& header, std::vector<char> const& body,
           asio::yield_context ctx) {
        header.size = body.size ();

        std::array<asio::const_buffer, 2> const buffers = {
            asio::const_buffer (&header, sizeof (header)),
            asio::const_buffer (body.data (), body.size ())};

        boost::fusion::for_each (header, [](auto& field) {
            boost::endian::native_to_little_inplace (field);
        });

        auto const bytes_written =
            asio::async_write (*stream_, buffers, ctx[ec_]);
        return (!ec_ && (bytes_written == (sizeof (header) + body.size ())));
    }

    bool
    write (MessageHeader header, Message const& message, asio::yield_context ctx) {
        return boost::apply_visitor (
            [this, &header, ctx](auto const& body) {
                return this->do_write (header, body, ctx);
            },
            message.body ());
    }

private:
    AsyncWriteStream* stream_;
    flatbuffers::FlatBufferBuilder fbb_;
    boost::system::error_code ec_;
};
