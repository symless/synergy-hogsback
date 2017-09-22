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
    LogManager::debug("retry!");
    setMode(ErrorViewMode::kNone);
}

QString
ErrorView::message() const
{
    switch (m_mode) {
    case ErrorViewMode::kCloudError:
        return "Cloud error";
    case ErrorViewMode::kServiceError:
        return "Service error";
    default:
        return "";
    }
}
