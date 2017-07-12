#include "WampClient.h"

#include <autobahn/autobahn.hpp>
#include <boost/asio.hpp>
#include <boost/version.hpp>
#include <chrono>
#include <iostream>
#include <memory>
#include <string>
#include <tuple>

WampClient::WampClient(boost::asio::io_service& ioService) :
    m_ioService(ioService),
    m_work()
{

}

void
WampClient::run(std::string ip, int port, bool debug)
{
    try {
        auto transport = std::make_shared<autobahn::wamp_tcp_transport>(
                m_ioService, boost::asio::ip::tcp::endpoint
                    (boost::asio::ip::address_v4::from_string(ip), port), debug);
        m_session = std::make_shared<autobahn::wamp_session>(m_ioService, debug);
        transport->attach(std::static_pointer_cast<autobahn::wamp_transport_handler>(m_session));

        boost::future<void> connect_future;
        boost::future<void> start_future;
        boost::future<void> join_future;
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

            start_future = m_session->start().then([&](boost::future<void> started) {
                try {
                    started.get();
                } catch (const std::exception& e) {
                    std::cerr << e.what() << std::endl;
                    m_ioService.stop();
                    return;
                }

                std::cerr << "session started" << std::endl;

                join_future = m_session->join("default").then([&](boost::future<uint64_t> joined) {
                    try {
                        std::cerr << "joined realm: " << joined.get() << std::endl;
                    } catch (const std::exception& e) {
                        std::cerr << e.what() << std::endl;
                        m_ioService.stop();
                        return;
                    }
                });
            });
        });

        std::cerr << "starting io service" << std::endl;
        m_work.reset(new boost::asio::io_service::work(m_ioService));
        m_ioService.run();
    }
    catch (const std::exception& e) {
        std::cerr << e.what() << std::endl;
        throw;
    }
}

void
WampClient::startCore(std::vector<std::string> cmd)
{
    autobahn::wamp_call_options call_options;
    call_options.set_timeout(std::chrono::seconds(10));

    boost::future<void> call_future;
    call_future = m_session->call("startCore", cmd, call_options).then(
    [&](boost::future<autobahn::wamp_call_result> result) {
        try {
            uint64_t sum = result.get().argument<uint64_t>(0);
            std::cerr << "call result: " << sum << std::endl;
        } catch (const std::exception& e) {
            std::cerr << "call failed: " << e.what() << std::endl;
            m_ioService.stop();
            return;
        }
    });
}

