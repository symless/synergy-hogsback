#include "catch.hpp"

#include "synergy/service/WebsocketSession.h"

#include <boost/asio/io_service.hpp>
#include <boost/signals2.hpp>
#include <string>

static const char* kPubSubServerHostname = "165.227.29.181";
static const char* kPubSubServerPort = "8081";

static boost::signals2::signal<void()> testFinished;
static bool timeout;

TEST_CASE("Nchan Websocket connections", "[Websocket]")
{
    timeout = false;
    std::string subResult;
    bool pubConnected = false;
    boost::asio::io_service ioService;

    WebsocketSession pubWebsocket(ioService, kPubSubServerHostname, kPubSubServerPort);
    WebsocketSession subWebsocket(ioService, kPubSubServerHostname, kPubSubServerPort);

    boost::asio::deadline_timer timer(ioService, boost::posix_time::seconds(5));
    timer.async_wait([](const boost::system::error_code&) {
        timeout = true;
        testFinished();
    });

    pubWebsocket.connected.connect(
        [&pubWebsocket, &pubConnected]() {
            pubConnected = true;
            std::string pubStr("hello world");
            pubWebsocket.write(pubStr);
        },
        boost::signals2::at_front
    );

    subWebsocket.messageReceived.connect(
        [&subResult](std::string msg) {
            subResult = std::move(msg);
            testFinished();
        },
        boost::signals2::at_front
    );

    testFinished.connect([&ioService]() {
        ioService.poll();
        ioService.stop();
    });

    pubWebsocket.connect("/synergy/pub/test");
    subWebsocket.connect("/synergy/sub/test");

    ioService.run();

    REQUIRE(timeout == false);
    REQUIRE(pubConnected == true);
    REQUIRE(subResult == "hello world");
}
