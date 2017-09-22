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

    Q_PROPERTY(bool enabled READ enabled NOTIFY modeChanged)
    Q_PROPERTY(QString message READ message NOTIFY modeChanged)

    bool enabled() const;
    ErrorViewMode mode() const;
    void setMode(const ErrorViewMode &mode);
    int retryTimeout() const;
    void setRetryTimeout(int value);
    Q_INVOKABLE void retry();
    QString message() const;

signals:
    void modeChanged();

public slots:

private:
    ErrorViewMode m_mode = ErrorViewMode::kNone;
    int m_retryTimeout = -1;
};
