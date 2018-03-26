#ifndef HOSTNAME_H
#define HOSTNAME_H

#include "LibMacro.h"

#include <QQmlEngine>
#include <QQuickItem>

class LIB_SPEC Hostname : public QQuickItem
{
	Q_OBJECT
    Q_DISABLE_COPY(Hostname)

public:

    static QObject* instance(QQmlEngine* engine = NULL, QJSEngine* scriptEngine = NULL);

    Hostname() = default;

    static QString local();

    Q_INVOKABLE QString QmlLocal();
};

#endif // HOSTNAME_H
