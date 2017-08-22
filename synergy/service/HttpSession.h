#ifndef HTTPSESSION_H
#define HTTPSESSION_H

#include "synergy/service/SecuredTcpSession.h"

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
    void onSessionConnected();
    void setupRequest(http::verb method, const std::string &target, const std::string &body = "");
    void onWriteFinished(errorCode ec);

private:
    SecuredTcpSession m_session;
    ssl::stream<tcp::socket>& m_stream;
    http::request<http::string_body> m_request;
    http::response<http::string_body> m_response;
};

#endif // HTTPSESSION_H
