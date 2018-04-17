#ifndef PROXYSTREAM_H
#define PROXYSTREAM_H

#include <string>
#include <stdexcept>
#include <type_traits>
#include <boost/asio/ip/tcp.hpp>
#include <boost/asio/connect.hpp>
#include <boost/asio/async_result.hpp>
#include <boost/beast/core.hpp>
#include <boost/beast/http.hpp>
#include <fmt/format.h>

template <typename NextLayer>
class ProxyClient {
public:
    using next_layer_type = NextLayer;
    using lowest_layer_type = typename next_layer_type::lowest_layer_type;

    template <typename NextLayerInit>
    explicit ProxyClient (NextLayerInit&&, std::string host = "",
                          int port = 80);

    bool enabled() const noexcept;
    lowest_layer_type& lowest_layer() noexcept;

    template <typename Handler>
    auto
    async_connect (boost::asio::ip::tcp::resolver::iterator, Handler&&) ->
        typename boost::asio::async_result<std::decay_t<Handler>>::type;


    template <typename Buffer>
    std::size_t
    read_some (Buffer&&, boost::system::error_code& ec);

    template <typename Buffer>
    std::size_t
    write_some (Buffer const&, boost::system::error_code& ec);

    template <typename Buffer, typename Handler>
    void async_read_some (Buffer&&, Handler&&);

    template <typename Buffer, typename Handler>
    void async_write_some (Buffer const&, Handler&&);

private:
    next_layer_type m_nextLayer;
    std::string m_host;
    int m_port = 80;
    bool m_connected = false;
};

template<typename NextLayer> inline
bool
ProxyClient<NextLayer>::enabled() const noexcept {
    return !m_host.empty();
}

template<typename NextLayer> inline
typename ProxyClient<NextLayer>::lowest_layer_type&
ProxyClient<NextLayer>::lowest_layer() noexcept {
    return m_nextLayer.lowest_layer();
}

template<typename NextLayer>
bool
ProxyClient<NextLayer>::setProxy(std::string host, int port) {
    if ((port < 0) || (port > 65535)) {
        return false;
    } else if (!port) {
        port = 80;
    }
    bool const changed = (std::tie (host, port) != std::tie (m_host, m_port));
    if (changed) {
        m_host = std::move (host);
        m_port = port;
        m_connected = false;
        if (m_nextLayer.is_open()) {
            boost::system::error_code ec;
            m_nextLayer.close(ec);
        }
    }
    return changed;
}

template <typename NextLayer>
template <typename NextLayerInit> inline
ProxyClient<NextLayer>::ProxyClient (NextLayerInit&& nextLayer,
                                     std::string host, int const port):
    m_nextLayer (std::forward<NextLayerInit>(nextLayer)),
    m_host (std::move (host)),
    m_port (port) {
}

template <typename NextLayer>
template <typename Handler> inline
auto
ProxyClient<NextLayer>::async_connect
(boost::asio::ip::tcp::resolver::iterator dnsIt, Handler&& handler) ->
    typename boost::asio::async_result<std::decay_t<Handler>>::type
{
    boost::asio::async_result<std::decay_t<Handler>> result (handler);

    if (m_connected) {
        throw std::runtime_error ("connect() called on a proxy client that's "
                                  "lready connected");
    }

    if (dnsIt == boost::asio::ip::tcp::resolver::iterator()) {
        // TODO
    }

    if (this->enabled()) {
        auto proxy = boost::asio::ip::tcp::endpoint (
                        boost::asio::ip::address_v4::from_string(m_host),
                        m_port
                      );
        auto target = fmt::format ("{}:{}", dnsIt->host_name(),
                                   dnsIt->service_name());

        m_nextLayer.async_connect (proxy, [this, target, handler](auto ec) {
            using boost::asio::asio_handler_invoke;
            asio_handler_invoke ([this, target, handler, ec]() mutable {
                if (!ec) {
                    boost::beast::http::request<boost::beast::http::empty_body> req;
                    req.version (11);
                    req.method (boost::beast::http::verb::connect);
                    req.target (target);

                    boost::beast::http::write (m_nextLayer, req, ec);
                    if (ec) {
                        handler (ec);
                        return;
                    }

                    boost::beast::multi_buffer mb;
                    boost::beast::http::response_parser<boost::beast::http::empty_body> resp;
                    resp.skip(true);
                    boost::beast::http::read (m_nextLayer, mb, resp, ec);
                    if (!ec) {
                        m_connected = true;
                    }
                    handler (ec);
                }
            }, &handler);
        });
    } else {
        boost::asio::async_connect (m_nextLayer, dnsIt, [this, handler](auto ec, auto it) {
            using boost::asio::asio_handler_invoke;
            asio_handler_invoke ([this, handler, ec]() {
                this->m_connected = true;
                handler (ec);
            }, &handler);
        });
    }

    return result.get();
}

template <typename NextLayer>
template <typename Buffer> inline
std::size_t
ProxyClient<NextLayer>::read_some (Buffer&& buffer,
                                   boost::system::error_code& ec) {
    if (!m_connected) {
        throw std::runtime_error ("Proxy client must be connected before "
                                  "invoking a read operation");
    }

    return m_nextLayer.read_some (buffer, ec);
}

template <typename NextLayer>
template <typename Buffer, typename Handler> inline
void
ProxyClient<NextLayer>::async_read_some (Buffer&& buffer, Handler&& handler) {
    if (!m_connected) {
        throw std::runtime_error ("Proxy client must be connected before "
                                  "invoking a read operation");
    }
    return m_nextLayer.async_read_some (std::forward<Buffer>(buffer),
                                        std::forward<Handler>(handler));
}

template <typename NextLayer>
template <typename Buffer, typename Handler> inline
void
ProxyClient<NextLayer>::async_write_some (Buffer const& buffer,
                                          Handler&& handler) {
    if (!m_connected) {
        throw std::runtime_error ("Proxy client must be connected before "
                                  "invoking a write operation");
    }

    return m_nextLayer.async_write_some (buffer,
                                         std::forward<Handler>(handler));
}

template <typename NextLayer>
template <typename Buffer> inline
std::size_t
ProxyClient<NextLayer>::write_some (Buffer const& buffer,
                                    boost::system::error_code& ec) {
    if (!m_connected) {
        throw std::runtime_error ("Proxy client must be connected before "
                                  "invoking a write operation");
    }

    return m_nextLayer.write_some (buffer, ec);
}

#endif // PROXYSTREAM_H
