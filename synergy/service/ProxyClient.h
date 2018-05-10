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
#include <boost/algorithm/string/join.hpp>
#include <fmt/format.h>
#include <fmt/ostream.h>

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

    bool setProxy (std::string host, int port = 80);

private:
    next_layer_type m_nextLayer;
    boost::asio::ip::tcp::resolver m_resolver;
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
        if (m_nextLayer.is_open()) {
            boost::system::error_code ec;
            m_nextLayer.close(ec);
        }
        m_connected = false;
    }
    return changed;
}

template <typename NextLayer>
template <typename NextLayerInit> inline
ProxyClient<NextLayer>::ProxyClient (NextLayerInit&& nextLayer,
                                     std::string host, int const port):
    m_nextLayer (std::forward<NextLayerInit>(nextLayer)),
    m_resolver (m_nextLayer.get_io_service()),
    m_host (std::move (host)),
    m_port (port) {
}

template <typename NextLayer>
template <typename Handler> inline
auto
ProxyClient<NextLayer>::async_connect
(boost::asio::ip::tcp::resolver::iterator serverItr, Handler&& handler) ->
    typename boost::asio::async_result<std::decay_t<Handler>>::type
{
    using boost::asio::asio_handler_invoke;
    boost::asio::async_result<std::decay_t<Handler>> result (handler);

    if (m_connected) {
        throw std::runtime_error ("connect() called on a proxy client that's "
                                  "already connected");
    }

    if (serverItr == boost::asio::ip::tcp::resolver::iterator()) {
        asio_handler_invoke ([this, handler]() {
            handler (boost::asio::error::host_not_found);
        }, &handler);
    } else if (this->enabled()) {
        auto target = fmt::format ("{}:{}", serverItr->host_name(),
                                   serverItr->service_name());

        // TODO: Using synchronous read and write operations in this handler
        //       is a bad idea. To maintain the illusion of a regular stream
        //       these should be properly composed async operations with timeouts
        auto connect_handler = [this, target, handler](auto ec) {
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
                    resp.skip (true);
                    boost::beast::http::read (m_nextLayer, mb, resp, ec);
                    if (!ec) {
                        m_connected = true;
                    }
                    handler (ec);
                }
            }, &handler);
        };

        // Test if the proxy hostname is an IP string or a hostname by
        // attempting a conversion.
        boost::system::error_code ec;
        auto ip = boost::asio::ip::address_v4::from_string(m_host, ec);

        if (ec) {
            // If we're dealing with a hostname we need to resolve before we can
            // continue. Ideally we'd want to do this in setProxy() but then
            // we'd somehow need to block any connect() call until the result
            // was in.
            using query_type = decltype(m_resolver)::query;

            serviceLog()->warn("Resolving proxy host name '{}'...", m_host);
            m_resolver.async_resolve (query_type(m_host, std::to_string(m_port),
                                      query_type::flags::numeric_service
                                    | query_type::flags::address_configured),
            [this, handler, connect_handler](auto ec, auto proxyIt) {
                asio_handler_invoke ([this, handler, connect_handler, ec,
                                     proxyIt]() {
                    std::vector<std::string> addresses;

                    std::transform (proxyIt, decltype(proxyIt)(),
                                    std::back_inserter(addresses), [](auto& re) {
                        return re.endpoint().address().to_string();
                    });

                    serviceLog()->warn("Proxy hostname ('{}') resolved to {}. Connecting...",
                                       m_host, boost::algorithm::join (addresses, ", "));

                    if (ec) {
                        connect_handler (ec);
                    } else {
                        boost::asio::async_connect (m_nextLayer, proxyIt,
                            [this, handler, connect_handler](auto ec, auto) {
                            asio_handler_invoke ([this, connect_handler, ec]() {
                                connect_handler (ec);
                            }, &handler);
                        });
                    }
                }, &handler);
            });
        } else {
            serviceLog()->warn("Connecting to proxy at {}...", m_host);
            auto proxy = boost::asio::ip::tcp::endpoint (ip, m_port);
            m_nextLayer.async_connect (proxy, std::move (connect_handler));
        }
    } else {
        boost::asio::async_connect (m_nextLayer, serverItr,
                                    [this, handler](auto ec, auto it) {
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
    return m_nextLayer.read_some (buffer, ec);
}

template <typename NextLayer>
template <typename Buffer, typename Handler> inline
void
ProxyClient<NextLayer>::async_read_some (Buffer&& buffer, Handler&& handler) {
    return m_nextLayer.async_read_some (std::forward<Buffer>(buffer),
                                        std::forward<Handler>(handler));
}

template <typename NextLayer>
template <typename Buffer, typename Handler> inline
void
ProxyClient<NextLayer>::async_write_some (Buffer const& buffer,
                                          Handler&& handler) {
    return m_nextLayer.async_write_some (buffer,
                                         std::forward<Handler>(handler));
}

template <typename NextLayer>
template <typename Buffer> inline
std::size_t
ProxyClient<NextLayer>::write_some (Buffer const& buffer,
                                    boost::system::error_code& ec) {
    return m_nextLayer.write_some (buffer, ec);
}

#endif // PROXYSTREAM_H
