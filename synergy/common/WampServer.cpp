#include "WampServer.h"

#include "WampRouter.h"

#include <iostream>
#include <autobahn/autobahn.hpp>
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

WampServer::WampServer()
{
}

void WampServer::start(std::string ip, int port, bool debug)
{
    auto session = std::make_shared<autobahn::wamp_session>(m_ioService, debug);
    auto transport = std::make_shared<autobahn::wamp_tcp_transport>
                        (m_ioService, tcp::endpoint
                            (ip::address_v4::from_string(ip), port), debug);

    boost::future<void> connect_future;
    boost::future<void> start_future;
    boost::future<void> join_future;
    boost::future<void> provide_future_add;

    transport->attach (std::static_pointer_cast<autobahn::wamp_transport_handler>(session));
    connect_future = transport->connect().then ([&](boost::future<void> connected) {
        try {
            connected.get();
        } catch (const std::exception& e) {
            std::cerr << e.what() << std::endl;
            return;
        }
        std::cout << "Connected!" << std::endl;

        start_future = session->start().then([&](boost::future<void> started) {
        try {
            started.get();
        } catch (const std::exception& e) {
            std::cerr << e.what() << std::endl;
            m_ioService.stop();
            return;
        }

        std::cerr << "session started" << std::endl;

        join_future = session->join("default").then([&](boost::future<uint64_t> joined) {
            try {
                std::cerr << "joined realm: " << joined.get() << std::endl;
            } catch (const std::exception& e) {
                std::cerr << e.what() << std::endl;
                m_ioService.stop();
                return;
            }

//            provide_future_add = session->provide("com.examples.calculator.add2", &add2).then(
//                [&](boost::future<autobahn::wamp_registration> registration) {
//                try {
//                    std::cerr << "registered procedure:" << registration.get().id() << std::endl;
//                } catch (const std::exception& e) {
//                    std::cerr << e.what() << std::endl;
//                    io.stop();
//                    return;
//                }
//            });

            provide_future_add = session->provide ("add", [](autobahn::wamp_invocation invocation) {
               auto a = invocation->argument<uint64_t>(0);
               auto b = invocation->argument<uint64_t>(1);

               std::cerr << "Procedure add invoked: " << a << ", " << b << std::endl;

               invocation->result(std::make_tuple(a + b));
            }).then(
                [&](boost::future<autobahn::wamp_registration> registration) {
                try {
                    std::cerr << "registered procedure:" << registration.get().id() << std::endl;
                } catch (const std::exception& e) {
                    std::cerr << e.what() << std::endl;
                    m_ioService.stop();
                    return;
                }
            });

            });
        });
    });

    m_ioService.run();
}
