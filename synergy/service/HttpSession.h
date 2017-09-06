#ifndef HTTPSESSION_H
#define HTTPSESSION_H

#include "synergy/service/SecuredTcpClient.h"

#include <boost/beast/http.hpp>
#include <boost/asio/io_service.hpp>

namespace http = boost::beast::http;

class HttpSession final
{
public:
    HttpSession(boost::asio::io_service& ioService, std::string hostname, std::string port);

    void get(const std::string& target);
    void post(const std::string& target, const std::string &body);

private:
    void connect();
    void onTcpClientConnected();
    void setupRequest(http::verb method, const std::string &target, const std::string &body = "");
    void onWriteFinished(errorCode ec);

private:
    SecuredTcpClient m_tcpClient;
    http::request<http::string_body> m_request;
    http::response<http::string_body> m_response;
};

#endif // HTTPSESSION_H
