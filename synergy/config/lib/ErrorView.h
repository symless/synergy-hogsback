#pragma once

#include <QObject>

class ErrorView : public QObject
{
    Q_OBJECT
public:
    explicit ErrorView(QObject *parent = 0);

    Q_PROPERTY(bool enabled READ enabled WRITE setEnabled NOTIFY enabledChanged)

    bool enabled() const;
    void setEnabled(bool enabled);

signals:
    void enabledChanged();

public slots:

private:
    bool m_enabled;
};
