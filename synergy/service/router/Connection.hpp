#pragma once
#include "Asio.hpp"
#include "Message.hpp"
#include "MessageReader.hpp"
#include "MessageWriter.hpp"
#include <boost/signals2/signal.hpp>
#include <boost/asio/ssl/stream.hpp>
#include <functional>
#include <deque>
#include <memory>

using namespace synergy::protocol::v2;
class Message;

class Connection final : public std::enable_shared_from_this<Connection> {
public:
    using ssl_stream = boost::asio::ssl::stream<boost::asio::ip::tcp::socket&>;

    Connection (tcp::socket&& socket, boost::asio::ssl::context& context);
    ~Connection () noexcept;
    uint32_t id () const noexcept;
    bool start (ssl_stream::handshake_type, asio::yield_context ctx);
    void stop ();
    bool send (Message const&);
    bool send (MessageHeader const&, Message const&);
    tcp::endpoint endpoint () const;

private:
    static uint32_t next_connection_id_;
    uint32_t id_;
    tcp::socket socket_;
    tcp::endpoint endpoint_;
    ssl_stream stream_;
    MessageReader<ssl_stream> reader_;
    MessageWriter<ssl_stream> writer_;
    std::deque<std::pair<MessageHeader, Message>> message_queue_;
    bool enabled_ = false;

public:
    template <typename... Args>
    using signal = boost::signals2::signal<Args...>;

    signal<void(std::shared_ptr<Connection>)> on_connected;
    signal<void(std::shared_ptr<Connection>)> on_disconnect;
    signal<void(MessageHeader const&, Message&, std::shared_ptr<Connection>)>
        on_message;
};

inline uint32_t
Connection::id () const noexcept {
    return id_;
}
