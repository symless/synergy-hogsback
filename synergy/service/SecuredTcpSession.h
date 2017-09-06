#ifndef TCPSESSION_H
#define TCPSESSION_H

#include <boost/asio/ip/tcp.hpp>
#include <boost/asio/ssl/stream.hpp>
#include <boost/asio/io_service.hpp>
#include <boost/beast/core.hpp>
#include <boost/signals2.hpp>

using tcp = boost::asio::ip::tcp;
namespace ssl = boost::asio::ssl;
typedef boost::system::error_code errorCode;

class SecuredTcpSession
{
public:
    SecuredTcpSession(boost::asio::io_service& ioService,  boost::asio::ssl::context& context);
    virtual ~SecuredTcpSession();

    ssl::stream<tcp::socket>& stream();

    template <typename... Args>
    using signal = boost::signals2::signal<Args...>;

    signal<void(SecuredTcpSession*)> connected;
    signal<void(SecuredTcpSession*)> connectFailed;

    void startSslHandshake(bool serverMode);
    void onSslHandshakeFinished(errorCode ec);

private:
    boost::asio::io_service& m_ioService;
    boost::asio::ssl::context& m_sslContext;
    ssl::stream<tcp::socket> m_stream;
};

#endif // TCPSESSION_H
