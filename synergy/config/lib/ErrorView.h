#pragma once

#include <QObject>

enum class ErrorViewMode
{
    kNone,
    kCloudError,
    kServiceError
};

class ErrorView : public QObject
{
    Q_OBJECT
public:
    explicit ErrorView(QObject *parent = 0);

    Q_PROPERTY(bool enabled READ enabled WRITE setEnabled NOTIFY enabledChanged)

    bool enabled() const;
    ErrorViewMode mode() const;
    void setMode(const ErrorViewMode &mode);
    int retryTimeout() const;
    void setRetryTimeout(int value);

signals:
    void enabledChanged();

public slots:

private:
    void setEnabled(bool enabled);

private:
    bool m_enabled = false;
    ErrorViewMode m_mode = ErrorViewMode::kNone;
    int m_retryTimeout = -1;
};
