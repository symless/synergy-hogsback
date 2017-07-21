#include "WampServer.h"

#include "WampRouter.h"

#include <iostream>
#include <boost/thread/future.hpp>
#include <bonefish/native/native_server.hpp>
#include <bonefish/rawsocket/rawsocket_server.hpp>
#include <bonefish/rawsocket/tcp_listener.hpp>
#include <bonefish/router/wamp_routers.hpp>
#include <bonefish/serialization/wamp_serializers.hpp>
#include <bonefish/serialization/msgpack_serializer.hpp>
#include <bonefish/trace/trace.hpp>
#include <memory>

namespace ip =  boost::asio::ip;
using tcp = ip::tcp;

WampServer::WampServer(boost::asio::io_service& io, std::string ip, int port, bool debug) :
    m_executor(io)
{
    m_session = std::make_shared<autobahn::wamp_session>(io, debug);
    m_transport = std::make_shared<autobahn::wamp_tcp_transport>
                        (io, tcp::endpoint
                            (ip::address_v4::from_string(ip), port), debug);
    m_transport->attach (std::static_pointer_cast<autobahn::wamp_transport_handler>(m_session));
}

void WampServer::start()
{
    boost::future<void> connect_future = m_transport->connect().then (m_executor, [&](boost::future<void> connected) {
        try {
            connected.get();
        } catch (const std::exception& e) {
            std::cerr << e.what() << std::endl;
            return;
        }
        std::cout << "Connected!" << std::endl;

        boost::future<void> start_future = m_session->start().then(m_executor, [&](boost::future<void> started) {
        try {
            started.get();
        } catch (const std::exception& e) {
            std::cerr << e.what() << std::endl;
            //io.stop();
            return;
        }

        std::cerr << "session started" << std::endl;

        boost::future<void> join_future = m_session->join("default").then(m_executor, [&](boost::future<uint64_t> joined) {
            try {
                std::cerr << "joined realm: " << joined.get() << std::endl;
            } catch (const std::exception& e) {
                std::cerr << e.what() << std::endl;
                //io.stop();
                return;
            }

            boost::future<void> provide_future_add = m_session->provide ("startCore", [this](autobahn::wamp_invocation invocation) {
               auto args = invocation->arguments<std::vector<std::vector<std::string>>>();
               startCore(args[0]);
               invocation->empty_result();
            }).then(
                m_executor,
                [&](boost::future<autobahn::wamp_registration> registration) {
                try {
                    std::cerr << "registered procedure:" << registration.get().id() << std::endl;
                } catch (const std::exception& e) {
                    std::cerr << e.what() << std::endl;
                    //io.stop();
                    return;
                }
            });

            });
        });
    });
}
