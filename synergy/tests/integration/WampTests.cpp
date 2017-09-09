#include "catch.hpp"

#include "synergy/common/RpcManager.h"
#include "synergy/common/WampServer.h"
#include "synergy/common/WampClient.h"

#include <boost/date_time/posix_time/posix_time.hpp>
#include <boost/asio/io_service.hpp>
#include <boost/signals2.hpp>
#include <vector>

boost::signals2::signal<void()> serverReady;
static boost::signals2::signal<void()> testFinished;
std::vector<std::string> testCommand;
static bool timeout;

void startTest(std::vector<std::string> cmd)
{
    testCommand = std::move(cmd);
    testFinished();
}

void
provideCoreTest(RpcManager& rpcManager)
{
    auto server = rpcManager.server();

    server->provide("synergy.core.start.test",
                     [&rpcManager](std::vector<std::string>& cmd) {
        startTest(std::move(cmd));
    });

    serverReady();
}

TEST_CASE("WAMP RPC core start call is recieved", "[Wamp]")
{
    timeout = false;
    boost::asio::io_service ioService;
    RpcManager rpcManager(ioService);
    rpcManager.ready.connect([&rpcManager]() {
        provideCoreTest(rpcManager);
    });

    WampClient wampClient(ioService);
    serverReady.connect([&wampClient, &rpcManager]() {
        wampClient.connected.connect([&wampClient]() {
            std::vector<std::string> testCommand;
            testCommand.push_back("cmd");
            testCommand.push_back("--arg1");
            wampClient.call<void> ("synergy.core.start.test", testCommand);
        });
        wampClient.start(rpcManager.ipAddress(), rpcManager.port());
    });

    testFinished.connect([&ioService]() {
        ioService.poll();
        ioService.stop();
    });

    boost::asio::deadline_timer timer(ioService, boost::posix_time::seconds(5));
    timer.async_wait([](const boost::system::error_code&) {
        timeout = true;
        testFinished();
    });

    rpcManager.start();

    ioService.run();

    REQUIRE(timeout == false);
    REQUIRE(testCommand.size() == 2);
    REQUIRE(testCommand[0] == "cmd");
    REQUIRE(testCommand[1] == "--arg1");
}
