#pragma once
#include "Asio.hpp"
#include "Message.hpp"
#include "MessageReader.hpp"
#include "MessageWriter.hpp"
#include <boost/signals2/signal.hpp>
#include <boost/asio/ssl/stream.hpp>
#include <functional>
#include <memory>

using namespace synergy::protocol::v2;
namespace ssl = boost::asio::ssl;

class Message;

class Connection final : public std::enable_shared_from_this<Connection> {
    template <typename... Args>
    using signal = boost::signals2::signal<Args...>;

public:
    explicit Connection (tcp::socket socket);
    ~Connection () noexcept;

    uint32_t id () const noexcept;
    void start (bool fromServer);
    void stop ();
    bool send (Message const&, asio::yield_context ctx);
    bool send (MessageHeader const&, Message const&, asio::yield_context ctx);
    tcp::endpoint endpoint () const;

private:
    void loadRawCertificate();

    static const std::string sslCertificate();
    static const std::string sslKey();
    static const std::string sslDH();

private:
    using SslStream = ssl::stream<boost::asio::ip::tcp::socket&>;
    static uint32_t next_connection_id_;
    uint32_t id_;
    tcp::socket socket_;
    tcp::endpoint endpoint_;
    ssl::context context_;
    SslStream stream_;
    MessageReader<SslStream> reader_;
    MessageWriter<SslStream> writer_;
    bool enabled_ = false;

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
