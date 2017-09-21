#include "ErrorView.h"

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
    m_enabled = enabled;
}
