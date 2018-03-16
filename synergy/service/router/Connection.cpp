#include "Connection.hpp"
#include "Message.hpp"
#include "MessageReader.hpp"
#include "MessageWriter.hpp"
#include <fmt/format.h>
#include <iostream>
#include <synergy/common/SocketOptions.hpp>
#include <synergy/service/ServiceLogs.h>

uint32_t Connection::next_connection_id_ = 0;

Connection::Connection (tcp::socket&& socket,
                        boost::asio::ssl::context& context):
        id_ (++next_connection_id_),
        socket_ (std::move (socket)),
        remote_endpoint_ (socket_.remote_endpoint ()),
        stream_ (socket_, context),
        reader_ (stream_),
        writer_ (stream_)
{
    routerLog ()->debug ("Connection {} created", id ());

    boost::system::error_code ec;
    socket_.set_option (tcp::no_delay (true), ec);
    if (ec) {
        routerLog ()->warn ("Failed to disable Nagles algorithm on connection"
                            " {}", id ());
    }

    if (!set_tcp_keep_alive_options (socket_, 10, 1, 10)) {
        routerLog ()->warn ("Failed to enable TCP keepalive on connection {}",
                            id ());
    }

    if (!set_socket_to_close_on_exec (socket_, ec)) {
        routerLog ()->critical ("Failed to set socket to close-on-exec for "
                                "connection {}: {}", id (), ec.message());
    }
}

Connection::~Connection () noexcept {
    routerLog ()->debug ("Connection {} destroyed", id ());
}

void
Connection::start () {
    asio::spawn (
        socket_.get_io_service (),
        [ this, self = this->shared_from_this () ](auto ctx) {
            enabled_ = true;
            while (true) {
                MessageHeader header;
                Message message;
                bool complete = false;

                if (reader_.read_header (header, ctx)) {
                    if (reader_.read_body (header, message, ctx)) {
                        complete = true;
                        on_message (header, message, self);
                    }
                }

                auto ec = reader_.error ();
                if ((ec == asio::error::operation_aborted) && !complete) {
                    routerLog()->error ("Message processing on connection {}, connected to {}, "
                                        "interrupted. Terminating", this->id (),
                                       this->remote_endpoint());
                    on_disconnect (self);
                    break;
                } else if (!enabled_) {
                    routerLog()->debug ("Connection {}, connected to {}, disabled"
                                        this->id (), this->remote_endpoint());
                    break;
                } else  if (ec) {
                    routerLog()->error ("Message processing on connection {}, connected "
                                        "to {}, failed: {} (ec = {})", this->id (), 
                                        this->remote_endpoint(), ec.message (), ec.value());
                    on_disconnect (self);
                    break;
                }
            }
            
            routerLog()->debug("Connection {}, connected to {},  terminated receive loop", 
                                this->id (), this->remote_endpoint());
    });

    on_connected (this->shared_from_this ());
}

void
Connection::stop () {
    if (!enabled_) {
        return;
    }

    enabled_ = false;
    socket_.cancel ();
    socket_.get_io_service().poll();
}

bool
Connection::send (Message const& message) {
    return send (message.header(), message);
}

bool
Connection::send (MessageHeader const& header, Message const& message) {
    message_queue_.emplace_back (header, message);

    if (message_queue_.size() == 1) {
        asio::spawn (socket_.get_io_service(),
                     [this, self = this->shared_from_this ()] (auto ctx) {
            while (!message_queue_.empty()) {
                auto& envelope = message_queue_.front();
                writer_.write (envelope.first, envelope.second, ctx);
                message_queue_.pop_front();
            }
        });
    }

    return true;
}

tcp::endpoint 
Connection::remote_endpoint() const
{
    return remote_endpoint_;
}

tcp::endpoint
Connection::remote_acceptor_endpoint () const {
    return tcp::endpoint (remote_endpoint_.address (), 24802);
}
