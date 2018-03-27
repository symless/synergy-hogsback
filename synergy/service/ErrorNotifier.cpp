#include <synergy/service/ErrorNotifier.h>

#include <synergy/service/CoreStatusMonitor.h>
#include <synergy/service/CoreErrorMonitor.h>
#include <synergy/service/RouterErrorMonitor.h>
#include <synergy/service/CloudClient.h>
#include <synergy/common/ProfileConfig.h>
#include <synergy/common/ScreenError.h>
#include <synergy/service/ServiceLogs.h>

ErrorNotifier::ErrorNotifier(CloudClient& cloudClient, ProfileConfig& profileConfig, UserConfig& userConfig) :
    m_cloudClient(cloudClient),
    m_profileConfig(profileConfig),
    m_userConfig(userConfig)
{
}

void ErrorNotifier::install(CoreErrorMonitor& monitor)
{
    monitor.error.connect([this](std::string const& screenName, ScreenError code, std::string const& message){
        Screen screen = m_profileConfig.getScreen(screenName);
        screen.setErrorCode(code);
        screen.setErrorMessage(message);

        m_cloudClient.updateScreenError(screen);
    });
}

void ErrorNotifier::install(CoreStatusMonitor &monitor)
{
    monitor.screenStatusChanged.connect([this](const int screenId, ScreenStatus state){
        if (state == ScreenStatus::kConnected) {
            Screen screen = m_profileConfig.getScreen(screenId);
            if (screen.errorCode() != ScreenError::kRouterUnreachableNode &&
                screen.errorCode() != ScreenError::kNone) {
                screen.setErrorCode(ScreenError::kNone);
                screen.setErrorMessage("");

                m_cloudClient.updateScreenError(screen);
            }
        }
    });
}

void ErrorNotifier::install(RouterErrorMonitor &monitor)
{
    monitor.screenReachable.connect([this](int64_t screen_id){
        Screen screen = m_profileConfig.getScreen(screen_id);

        if (screen.errorCode() == ScreenError::kRouterUnreachableNode) {
            serviceLog()->debug("Clearing screen {} error state", screen.id());
            screen.setErrorCode(ScreenError::kNone);
            screen.setErrorMessage("");
            m_cloudClient.updateScreenError(screen);
        } else {
            serviceLog()->debug("Ignoring reachable screen error update. Screen ID = {}, "
                                "Existing error = '{}'",
                                screen.id(), screen.errorMessage());
        }
    });

    monitor.screenUnreachable.connect([this](int64_t screen_id){
        Screen screen = m_profileConfig.getScreen(screen_id);
        Screen localScreen = m_profileConfig.getScreen(m_userConfig.screenId());

        // local machine is always reachable to itself
        if (screen.name() == localScreen.name()) {
            return;
        }

        screen.setErrorCode(ScreenError::kRouterUnreachableNode);
        std::string errorMessage;
        errorMessage = localScreen.name();
        errorMessage += " can't reach this screen within your network";
        screen.setErrorMessage(errorMessage);

        m_cloudClient.updateScreenError(screen);
    });
}
