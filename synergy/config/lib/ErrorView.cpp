#include <synergy/config/lib/ErrorView.h>

#include <synergy/config/lib/LogManager.h>

ErrorView::ErrorView(QObject *parent) :
    QObject(parent),
    m_enabled(false)
{
}

bool
ErrorView::enabled() const
{
    return m_enabled;
}

void
ErrorView::setEnabled(bool enabled)
{
    LogManager::debug(QString("error overlay enabled=%1").arg(enabled));
    m_enabled = enabled;
    emit enabledChanged();
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
    setEnabled(mode != ErrorViewMode::kNone);
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
