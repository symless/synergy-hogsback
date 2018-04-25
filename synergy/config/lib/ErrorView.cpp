#include <synergy/config/lib/ErrorView.h>
#include <synergy/config/lib/LogManager.h>
#include <synergy/config/lib/VersionManager.h>

ErrorView::ErrorView(QObject *parent) :
    QObject(parent)
{
}

bool
ErrorView::visible() const
{
    return m_mode != ErrorViewMode::kNone;
}

bool
ErrorView::retrying() const
{
    return m_retrying;
}

void
ErrorView::setRetrying(bool retrying)
{
    m_retrying = retrying;
    retryingChanged();
}

ErrorViewMode
ErrorView::mode() const
{
    return m_mode;
}

void
ErrorView::setMode(const ErrorViewMode &mode)
{
    m_mode = mode;
    setRetrying(false);
    emit modeChanged();
}

void
ErrorView::retry()
{
    setRetrying(true);
    retryRequested(m_mode);
}

QString
ErrorView::message() const
{
    switch (m_mode) {
        case ErrorViewMode::kCloudError:
            return "There was a problem connecting to the auto-config service.";
        case ErrorViewMode::kServiceError:
            return "There was a problem connecting to the background service.";
        default:
            return "";
    }
}

QString
ErrorView::help() const
{
    VersionManager* versionManager = qobject_cast<VersionManager*>(VersionManager::instance());

    switch (m_mode) {
        case ErrorViewMode::kCloudError:
            return QString("<a href='https://symless.com/synergy/help/connection/auto-config-service?source=s2-app&version=%1'>Help</a>").arg(versionManager->buildVersion());
        case ErrorViewMode::kServiceError:
            return QString("<a href='https://symless.com/synergy/help/connection/background-service?source=s2-app&version=%1'>Help</a>").arg(versionManager->buildVersion());
        default:
            return "";
    }
}