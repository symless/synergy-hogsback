#include <synergy/service/CoreProcess.h>
#include <synergy/common/UserConfig.h>
#include <synergy/common/ProfileConfig.h>

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

void repeatServerChangeFunc(const boost::system::error_code&,
                            ProfileConfig* profileConfig,
                            int& startCount, boost::asio::deadline_timer* timer)
{
    startCount++;
    if (startCount <= kMaxmiumStartTime) {
        profileConfig->profileServerChanged((startCount % 2) + 1);

        float randomDelay = kMinRestartDelay + static_cast<float>(rand()) / (static_cast<float>(RAND_MAX / (kMaxRestartDelay - kMinRestartDelay)));

        timer->expires_at(timer->expires_at() + boost::posix_time::seconds(randomDelay));
        timer->async_wait(boost::bind(repeatServerChangeFunc, boost::asio::placeholders::error, profileConfig, startCount, timer));
    }
}

TEST_CASE("Start and stop core process in different modes", "[CoreProcess]" ) {
    srand (static_cast <unsigned> (time(0)));

    boost::asio::io_service ioService;

    fakeit::Mock<UserConfig> userConfigMock;
    fakeit::Fake(Dtor(userConfigMock));
    Method(userConfigMock,screenId) = 1;

    auto profileConfig = std::make_shared<ProfileConfig>();

    CoreProcess coreProcess(ioService, std::shared_ptr<UserConfig>(&userConfigMock.get()), profileConfig);

    float finishTime = kMaxmiumStartTime * kMaxRestartDelay + kSignalDelay + kStartProcessPadding;
    boost::asio::deadline_timer timer(ioService, boost::posix_time::seconds(finishTime));

    int startCount = 0;
    boost::asio::deadline_timer signalDelayTimer(ioService, boost::posix_time::seconds(kSignalDelay));

    testFinished.connect([&ioService, &coreProcess]() {
        REQUIRE (coreProcess.currentServerId() == 1);
        REQUIRE (coreProcess.proccessMode() == ProcessMode::kServer);

        coreProcess.shutdown();

        ioService.poll();
        ioService.stop();
    });

    timer.async_wait([](const boost::system::error_code&) {
        testFinished();
    });

    signalDelayTimer.async_wait(boost::bind(repeatServerChangeFunc, boost::asio::placeholders::error, profileConfig.get(), startCount, &signalDelayTimer));

    ioService.run();
}

