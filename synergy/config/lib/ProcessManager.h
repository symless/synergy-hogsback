#ifndef PROCESSMANAGER_H
#define PROCESSMANAGER_H

#include "LibMacro.h"
#include "synergy/common/ScreenStatus.h"

#include <boost/asio.hpp>
#include <QQuickItem>
#include <QProcess>
#include <thread>
#include <memory>
#include <utility>

class ScreenListModel;
class AppConfig;
class WampClient;

class LIB_SPEC ProcessManager : public QQuickItem
{
    Q_OBJECT

public:
    ProcessManager();
    ProcessManager(WampClient&);
    ~ProcessManager();

signals:
    void screenStatusChanged(QPair<QString, ScreenStatus>);
    void rpcScreenStatusChanged(QString, int);
    void screenError(QString, int);
    void logCoreOutput(QString);
    void logServiceOutput(QString);

public slots:
    void onRpcScreenStatusChanged(QString, int);
    void onLogServiceOutput(QString text);
    void onLogCoreOutput(QString text);

private:
    std::shared_ptr<WampClient> m_wampClient;
};

#endif // PROCESSMANAGER_H
