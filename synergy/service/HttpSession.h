#ifndef HTTPSESSION_H
#define HTTPSESSION_H

#include "synergy/service/SecuredTcpClient.h"

#include <boost/beast/http.hpp>
#include <boost/asio/io_service.hpp>
#include <boost/signals2.hpp>

namespace http = boost::beast::http;

class HttpSession final
{
    friend class CloudClient;

public:
    HttpSession(boost::asio::io_service& ioService, std::string hostname, std::string port);

    void addHeader(std::string headerName, std::string headerContent);
    void get(const std::string& target);
    void post(const std::string& target, const std::string &body);

    template <typename... Args>
    using signal = boost::signals2::signal<Args...>;

    signal<void(http::status result, std::string response)> responseReceived;
    signal<void(errorCode ec)> requestFailed;

private:
    void send();
    void sendRequest();
    void onTcpClientConnected();
    void setupRequest(http::verb method, const std::string &target, const std::string &body = "");
    void onWriteFinished(errorCode ec);
    void onReadFinished(errorCode ec);

private:
    std::unique_ptr<SecuredTcpClient> m_tcpClient;
    std::map<std::string, std::string> m_headers;
    http::request<http::string_body> m_request;
    http::response<http::string_body> m_response;
    boost::beast::flat_buffer  m_readBuffer;
    boost::asio::io_service& m_ioService;
    std::string m_hostname;
    std::string m_port;
};

#endif // HTTPSESSION_H
