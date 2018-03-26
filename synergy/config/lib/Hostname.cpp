#include "Hostname.h"

#include <synergy/common/Hostname.h>

QObject*
Hostname::instance(QQmlEngine *engine, QJSEngine *scriptEngine)
{
    Q_UNUSED(engine)
    Q_UNUSED(scriptEngine)

    static Hostname s_instance;
    QQmlEngine::setObjectOwnership(&s_instance, QQmlEngine::CppOwnership);

    return &s_instance;
}

QString
Hostname::local()
{
    return QString::fromStdString(localHostname());
}
