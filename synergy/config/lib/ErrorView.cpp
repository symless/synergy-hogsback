#include <synergy/config/lib/ErrorView.h>

#include <synergy/config/lib/LogManager.h>

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
    switch (m_mode) {
        case ErrorViewMode::kCloudError:
            return "<a href='https://www.symless.com/synergy/help/connection/background-service'>Help</a>";
        case ErrorViewMode::kServiceError:
            return "<a href='https://www.symless.com/synergy/help/connection/auto-config-service'>Help</a>";
        default:
            return "";
    }
}