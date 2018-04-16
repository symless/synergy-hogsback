#include <synergy/service/router/Router.hpp>
#include <synergy/service/CoreProcess.h>
#include <synergy/common/UserConfig.h>
#include <synergy/common/ProfileConfig.h>
#include <synergy/common/ProcessCommand.h>
#include <synergy/common/Hostname.h>

#include <catch.hpp>
#include <fakeit.hpp>
#include <boost/asio/io_service.hpp>
#include <boost/signals2.hpp>
#include <memory>
#include <cstdlib>
#include <ctime>

static boost::signals2::signal<void()> testFinished;
const int kMaxmiumStartTime = 20;
const float kSignalDelay = 0.5f;
const float kMinRestartDelay = 0.05f;
const float kMaxRestartDelay = 0.3f;
const float kStartProcessPadding = 0.5f;
const int kTestNodePort = 24812;
const int kTestScreenId = 1;
void repeatServerChangeFunc(const boost::system::error_code&,
                            CoreProcess* coreProcess,
                            int& startCount, boost::asio::deadline_timer* timer)
{
    startCount++;
    if (startCount <= kMaxmiumStartTime) {
        int newId = (startCount % 2) + 1;

        if (newId == kTestScreenId) {
            coreProcess->startServer();
        }
        else {
            coreProcess->startClient(newId);
        }

        float randomDelay = kMinRestartDelay + static_cast<float>(rand()) / (static_cast<float>(RAND_MAX / (kMaxRestartDelay - kMinRestartDelay)));

        timer->expires_at(timer->expires_at() + boost::posix_time::seconds(randomDelay));
        timer->async_wait(boost::bind(repeatServerChangeFunc, boost::asio::placeholders::error, coreProcess, startCount, timer));
    }
}

TEST_CASE("Start and stop core process in different modes", "[CoreProcess]" ) {
    srand (static_cast <unsigned> (time(0)));

    boost::asio::io_service ioService;

    fakeit::Mock<UserConfig> userConfigMock;
    fakeit::Fake(Dtor(userConfigMock));
    Method(userConfigMock,screenId) = kTestScreenId;

    std::string jsonMock1 = R"JSON({"profile":{"id":1,"name":"mock1","server":1,"configVersion":1},"screens":[{"id":1,"name":"mocklocalhostname","x_pos":100,"y_pos":200,"version":1,"active":true,"status":"Connecting","ipList":"192.168.3.1,127.0.0.1","error_code":0,"error_message":""}]})JSON";
    auto profileConfig = std::make_shared<ProfileConfig>(ProfileConfig::fromJsonSnapshot(jsonMock1));


    Router router(ioService, kTestNodePort);
    fakeit::Mock<ProcessCommand> processCommandMock;
    fakeit::Fake(Dtor(processCommandMock));

    fakeit::When(Method(processCommandMock, serverCmd)).AlwaysDo([](auto name){
        ProcessCommand tempProcessCommand;
        std::vector<std::string> cmds = tempProcessCommand.serverCmd("mocklocalhostname");
 #ifdef __APPLE__
        cmds[0] = "synergy-core";
#endif
        return cmds;
    });
    fakeit::When(Method(processCommandMock, clientCmd)).AlwaysDo([](auto name){
        ProcessCommand tempProcessCommand;
        std::vector<std::string> cmds = tempProcessCommand.clientCmd("mocklocalhostname");
#ifdef __APPLE__
       cmds[0] = "synergy-core";
#endif
        return cmds;
    });

    CoreProcess coreProcess(ioService,
                            std::shared_ptr<UserConfig>(&userConfigMock.get()),
                            profileConfig,
                            std::shared_ptr<ProcessCommand>(&processCommandMock.get()));

    float finishTime = kMaxmiumStartTime * kMaxRestartDelay + kSignalDelay + kStartProcessPadding;
    boost::asio::deadline_timer timer(ioService, boost::posix_time::seconds(finishTime));

    int startCount = 0;
    boost::asio::deadline_timer signalDelayTimer(ioService, boost::posix_time::seconds(kSignalDelay));

    testFinished.connect([&ioService, &coreProcess]() {
        REQUIRE (coreProcess.currentServerId() == 1);
        REQUIRE (coreProcess.processMode() == ProcessMode::kServer);

        coreProcess.shutdown();
        ioService.poll();
        ioService.stop();
    });

    timer.async_wait([](const boost::system::error_code&) {
        testFinished();
    });

    signalDelayTimer.async_wait(boost::bind(repeatServerChangeFunc, boost::asio::placeholders::error, &coreProcess, startCount, &signalDelayTimer));

    ioService.run();
}

