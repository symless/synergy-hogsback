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
    using stream_type = boost::asio::ssl::stream<boost::asio::ip::tcp::socket&>;

    Connection (int64_t screenId, tcp::socket&& socket, boost::asio::ssl::context& context);
    ~Connection () noexcept;

    uint32_t id () const noexcept;
    stream_type& stream() noexcept;
    tcp::endpoint remote_acceptor_endpoint () const;

    void start ();
    void stop ();

    bool send (Message const&);
    bool send (MessageHeader const&, Message const&);

    int64_t screen_id() const;
    void set_screen_id(const int64_t &screen_id);

    tcp::endpoint endpoint() const;

private:
    static uint32_t next_connection_id_;
    uint32_t id_;
    tcp::socket socket_;
    int64_t screen_id_;

    /* We need this because once the socket has been disconnected you can no
     * longer call the remote_endpoint() function on it. */
    tcp::endpoint remote_endpoint_;

    stream_type stream_;
    MessageReader<stream_type> reader_;
    MessageWriter<stream_type> writer_;

    std::deque<std::pair<MessageHeader, Message>> message_queue_;
    bool enabled_ = false;

public:
    template <typename... Args>
    using signal = boost::signals2::signal<Args...>;

    signal<void(std::shared_ptr<Connection>)> on_connected;
    signal<void(std::shared_ptr<Connection>)> on_disconnect;
    signal<void(std::shared_ptr<Connection>)> identified;
    signal<void(MessageHeader const&, Message&, std::shared_ptr<Connection>)>
        on_message;
};

inline uint32_t
Connection::id () const noexcept {
    return id_;
}

inline Connection::stream_type&
Connection::stream() noexcept {
    return stream_;
}
