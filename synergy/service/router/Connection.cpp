#include "Connection.hpp"
#include "KeepAlive.hpp"
#include "Message.hpp"
#include "MessageReader.hpp"
#include "MessageWriter.hpp"
#include <fmt/format.h>
#include <iostream>
#include <synergy/service/ServiceLogs.h>

uint32_t Connection::next_connection_id_ = 0;

Connection::Connection (tcp::socket&& socket, ssl::context& context)
    : id_ (++next_connection_id_),
      socket_ (std::move (socket)),
      endpoint_ (socket_.remote_endpoint ()),
      stream_(socket_, context),
      reader_ (stream_),
      writer_ (stream_) {
    routerLog ()->debug ("Connection {} created", id ());

    boost::system::error_code ec;
    socket_.set_option (tcp::no_delay (true), ec);

    if (ec) {
        routerLog ()->warn (
            "Failed to disable Nagles algorithm on connection {}", id ());
    }

    if (!set_tcp_keep_alive_options (socket_, 5, 2, 10)) {
        throw std::runtime_error (fmt::format (
            "Failed to enable TCP keepalive on connection {}", id ()));
    }
}

Connection::~Connection () noexcept {
    routerLog ()->debug ("Connection {} destroyed", id ());
}

void
Connection::start (bool fromServer) {
    asio::spawn (
        socket_.get_io_service (),
        [ this, self = shared_from_this (), fromServer ](auto ctx) {
        boost::system::error_code ec;
        stream_.async_handshake(fromServer ? ssl::stream_base::server : ssl::stream_base::client, ctx[ec]);

        if (ec) {
            routerLog ()->error ("Connection {} failed in SSL handshake: {}",
                                this->id (), ec.message());
            on_disconnect (self);
        }
        else {
            enabled_ = true;
            on_connected (self);

            while (true) {
                MessageHeader header;
                Message message;

                if (reader_.read_header (header, ctx)) {
                    if (reader_.read_body (header, message, ctx)) {
                        on_message (header, message, self);
                    }
                }

                auto ec = reader_.error ();
                if (!enabled_ || (ec == asio::error::operation_aborted)) {
                    break;
                } else if ((ec == asio::error::connection_reset) ||
                           (ec == asio::error::eof) ||
                           (ec == asio::error::timed_out)) {
                    on_disconnect (self);
                    break;
                } else if (ec) {
                    throw boost::system::system_error (ec, ec.message ());
                }
            }

            routerLog ()->info ("Connection {} terminated receive loop",
                                this->id ());
        }
    });
}

void
Connection::stop () {
    enabled_ = false;
    boost::system::error_code ec;
    socket_.cancel (ec);
}

bool
Connection::send (Message const& message, asio::yield_context ctx) {
    auto header = message.make_header ();
    return send (header, message, ctx);
}

bool
Connection::send (MessageHeader const& header, Message const& message,
                  asio::yield_context ctx) {
    return writer_.write (header, message, ctx);
}

tcp::endpoint
Connection::endpoint () const {
    return tcp::endpoint (endpoint_.address (), 24802);
}
