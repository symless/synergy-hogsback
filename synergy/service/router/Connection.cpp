#include "Connection.hpp"
#include "KeepAlive.hpp"
#include "Message.hpp"
#include "MessageReader.hpp"
#include "MessageWriter.hpp"
#include <fmt/format.h>
#include <iostream>
#include <synergy/service/ServiceLogs.h>

uint32_t Connection::next_connection_id_ = 0;

Connection::Connection (tcp::socket socket)
    : id_ (++next_connection_id_), socket_ (std::move (socket)),
      endpoint_(socket_.remote_endpoint()), reader_(socket_), writer_(socket_) {
    routerLog()->info  ("Connection {} created", id ());

    boost::system::error_code ec;
    socket_.set_option (tcp::no_delay (true), ec);

    if (ec) {
        routerLog()->info  (
            "Failed to disable Nagles algorithm on connection {}",
            id ());
    }

    if (!set_tcp_keep_alive_options (socket_, 5, 2, 10)) {
        throw std::runtime_error (fmt::format (
            "Failed to enable TCP keepalive on connection {}", id ()));
    }
}

Connection::~Connection () noexcept {
    routerLog()->info ("Connection {} destroyed", id());
}

void
Connection::start () {
    enabled_ = true;

    /* Receive loop */
    asio::spawn (
        socket_.get_io_service (),
        [ this, self = shared_from_this () ](auto ctx) {
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

            routerLog()->info  (
                "Connection {} terminated receive loop", this->id ());
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
    auto header = message.make_header();
    return send (header, message, ctx);
}

bool
Connection::send (MessageHeader const& header, Message const& message,
                  asio::yield_context ctx) {
    return writer_.write (header, message, ctx);
}

tcp::endpoint
Connection::endpoint() const
{
    return tcp::endpoint (endpoint_.address(), 24802);
}
