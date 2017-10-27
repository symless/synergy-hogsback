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
namespace ssl = boost::asio::ssl;

class Message;

class Connection final : public std::enable_shared_from_this<Connection> {
    template <typename... Args>
    using signal = boost::signals2::signal<Args...>;

public:
    explicit Connection (tcp::socket &&socket, ssl::context& context);
    ~Connection () noexcept;

    uint32_t id () const noexcept;
    bool start (bool fromServer, asio::yield_context ctx);
    void stop ();
    bool send (Message const&);
    bool send (MessageHeader const&, Message const&);
    tcp::endpoint endpoint () const;

private:
    using SslStream = ssl::stream<boost::asio::ip::tcp::socket&>;
    static uint32_t next_connection_id_;
    uint32_t id_;
    tcp::socket socket_;
    tcp::endpoint endpoint_;
    SslStream stream_;
    MessageReader<SslStream> reader_;
    MessageWriter<SslStream> writer_;
    bool enabled_ = false;
    boost::asio::io_service::strand strand_;
    std::deque<std::pair<MessageHeader, Message>> messageQueue_;

public:
    signal<void(std::shared_ptr<Connection>)> on_connected;
    signal<void(std::shared_ptr<Connection>)> on_disconnect;
    signal<void(MessageHeader const&, Message&, std::shared_ptr<Connection>)>
        on_message;
};

inline uint32_t
Connection::id () const noexcept {
    return id_;
}
