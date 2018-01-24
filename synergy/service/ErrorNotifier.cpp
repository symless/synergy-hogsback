#include <synergy/service/ErrorNotifier.h>

#include <synergy/service/CoreStatusMonitor.h>
#include <synergy/service/CoreErrorMonitor.h>
#include <synergy/service/RouterErrorMonitor.h>
#include <synergy/service/CloudClient.h>
#include <synergy/common/ProfileConfig.h>
#include <synergy/common/ScreenError.h>

ErrorNotifier::ErrorNotifier(CloudClient& cloudClient, ProfileConfig& profileConfig) :
    m_cloudClient(cloudClient),
    m_profileConfig(profileConfig)
{
}

void ErrorNotifier::install(CoreErrorMonitor& monitor)
{
    monitor.error.connect([this](std::string const& screenName, ScreenError code, std::string const& message){
        Screen screen = m_profileConfig.getScreen(screenName);
        screen.setErrorCode(code);
        screen.setErrorMessage(message);
        screen.touch();

        m_cloudClient.updateScreen(screen);
    });
}

void ErrorNotifier::install(CoreStatusMonitor &monitor)
{
    monitor.screenStatusChanged.connect([this](std::string const& screenName, ScreenStatus state){
        if (state == ScreenStatus::kConnected) {
            Screen screen = m_profileConfig.getScreen(screenName);
            screen.setErrorCode(ScreenError::kNone);
            screen.setErrorMessage("");
            screen.touch();

            m_cloudClient.updateScreen(screen);
        }
    });
}

void ErrorNotifier::install(RouterErrorMonitor &monitor)
{
    monitor.screen_reached.connect([this](int64_t screen_id){
        Screen screen = m_profileConfig.getScreen(screen_id);
        screen.setErrorCode(ScreenError::kNone);
        screen.setErrorMessage("");
        screen.touch();

        m_cloudClient.updateScreen(screen);
    });

    monitor.screen_disjoint.connect([this](int64_t screen_id){
        Screen screen = m_profileConfig.getScreen(screen_id);
        screen.setErrorCode(ScreenError::kRouterUnreachableNode);
        screen.setErrorMessage("Can't reach this screen from your network");
        screen.touch();

        m_cloudClient.updateScreen(screen);
    });
}
