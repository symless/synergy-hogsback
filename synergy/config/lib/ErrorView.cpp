#include <synergy/config/lib/ErrorView.h>

#include <synergy/config/lib/LogManager.h>

ErrorView::ErrorView(QObject *parent) :
    QObject(parent)
{
}

bool
ErrorView::enabled() const
{
    return m_mode != ErrorViewMode::kNone;
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
    emit modeChanged();
}

int
ErrorView::retryTimeout() const
{
    return m_retryTimeout;
}

void
ErrorView::setRetryTimeout(int retryTimeout)
{
    m_retryTimeout = retryTimeout;
}

void
ErrorView::retry()
{
    retryRequested(m_mode);
}

QString
ErrorView::message() const
{
    switch (m_mode) {
    case ErrorViewMode::kCloudError:
        return "An error occurred while connecting to the config service.";
    case ErrorViewMode::kServiceError:
        return "An error occurred while connecting to the background service.";
    default:
        return "";
    }
}
