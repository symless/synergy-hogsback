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

    Q_PROPERTY(bool visible READ visible NOTIFY modeChanged)
    Q_PROPERTY(bool retrying READ retrying NOTIFY retryingChanged)
    Q_PROPERTY(QString message READ message NOTIFY modeChanged)

    bool visible() const;
    bool retrying() const;
    ErrorViewMode mode() const;
    void setMode(const ErrorViewMode &mode);
    Q_INVOKABLE void retry();
    QString message() const;

private:
    void setRetrying(bool retrying);

signals:
    void modeChanged();
    void retryingChanged();
    void retryRequested(ErrorViewMode mode);

public slots:

private:
    ErrorViewMode m_mode = ErrorViewMode::kNone;
    bool m_retrying = false;
};
