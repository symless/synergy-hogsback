#include "WampClient.h"

#include <autobahn/autobahn.hpp>
#include <boost/asio.hpp>
#include <boost/version.hpp>
#include <chrono>
#include <iostream>
#include <memory>
#include <string>
#include <tuple>

WampClient::WampClient(std::string ip, int port, bool debug)
{
    try {
        auto transport = std::make_shared<autobahn::wamp_tcp_transport>(
                m_ioService, boost::asio::ip::tcp::endpoint
                    (boost::asio::ip::address_v4::from_string(ip), port), debug);
        auto session = std::make_shared<autobahn::wamp_session>(m_ioService, debug);
        transport->attach(std::static_pointer_cast<autobahn::wamp_transport_handler>(session));

        boost::future<void> connect_future;
        boost::future<void> start_future;
        boost::future<void> join_future;
        boost::future<void> call_future;
        boost::future<void> leave_future;
        boost::future<void> stop_future;

        connect_future = transport->connect().then([&](boost::future<void> connected) {
            try {
                connected.get();
            } catch (const std::exception& e) {
                std::cerr << e.what() << std::endl;
                m_ioService.stop();
                return;
            }
            std::cerr << "transport connected" << std::endl;

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

                    autobahn::wamp_call_options call_options;
                    call_options.set_timeout(std::chrono::seconds(10));

                    std::tuple<uint64_t, uint64_t> arguments(23, 777);
                    call_future = session->call("add", arguments, call_options).then(
                    [&](boost::future<autobahn::wamp_call_result> result) {
                        try {
                            uint64_t sum = result.get().argument<uint64_t>(0);
                            std::cerr << "call result: " << sum << std::endl;
                        } catch (const std::exception& e) {
                            std::cerr << "call failed: " << e.what() << std::endl;
                            m_ioService.stop();
                            return;
                        }

                        leave_future = session->leave().then([&](boost::future<std::string> reason) {
                            try {
                                std::cerr << "left session (" << reason.get() << ")" << std::endl;
                            } catch (const std::exception& e) {
                                std::cerr << "failed to leave session: " << e.what() << std::endl;
                                m_ioService.stop();
                                return;
                            }

                            stop_future = session->stop().then([&](boost::future<void> stopped) {
                                std::cerr << "stopped session" << std::endl;
                                m_ioService.stop();
                            });
                        });
                    });
                });
            });
        });

        std::cerr << "starting io service" << std::endl;
        m_ioService.run();
        std::cerr << "stopped io service" << std::endl;

        transport->detach();
    }
    catch (const std::exception& e) {
        std::cerr << e.what() << std::endl;
        throw;
    }
}
