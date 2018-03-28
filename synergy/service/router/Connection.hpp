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

    Connection (tcp::socket&& socket, boost::asio::ssl::context& context);
    ~Connection () noexcept;

    uint32_t id () const noexcept;
    stream_type& stream() noexcept;

    boost::asio::ip::address local_ip() const;
    boost::asio::ip::address remote_ip() const;
    tcp::endpoint remote_acceptor_endpoint () const;

    void start ();
    void stop ();

    bool send (Message const&);
    bool send (MessageHeader const&, Message const&);

private:
    static uint32_t next_connection_id_;
    uint32_t id_;
    tcp::socket socket_;

    /* We need to store the remote because once the socket has been
     * disconnected you can no longer call the remote_endpoint() function on it.
     * Do the same for the local endpoint for good measure
     */
    boost::asio::ip::address remote_address_;
    boost::asio::ip::address local_address_;

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
