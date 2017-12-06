#pragma once

#include <synergy/common/WampClient.h>
#include <synergy/common/ScreenStatus.h>
#include <synergy/config/lib/LibMacro.h>

#include <boost/asio.hpp>
#include <QQuickItem>
#include <QProcess>
#include <thread>
#include <memory>
#include <utility>

class ScreenListModel;
class AppConfig;
class ErrorView;

class LIB_SPEC ServiceProxy : public QQuickItem
{
    Q_OBJECT

public:
    ServiceProxy();
    ~ServiceProxy();

    void start();
    void join();
    WampClient& wampClient();
    std::shared_ptr<ErrorView> errorView() const;
    void setErrorView(const std::shared_ptr<ErrorView>& errorView);
    void requestProfileSnapshot();
    void serverClaim(int screenId);

signals:
    void screenStatusChanged(QPair<QString, ScreenStatus>);
    void rpcScreenStatusChanged(QString, int);
    void screenError(QString, int);
    void logCoreOutput(QString);
    void logServiceOutput(QString);
    void receivedScreens(QByteArray reply);
    void rpcReceivedScreens(QString reply);
    void rpcCloudOffline();
    void rpcCloudOnline();
    void rpcAuthLogout();
    void rpcVersionCheck();
    void authLogout();

public slots:
    // TODO: figure out how to get rid of double signals hack
    // as wamp is running in a different thread, when it emits
    // signals it needs to first catch it in main thread then
    // emit another signal from main thread
    void onRpcReceivedScreens(QString);
    void onRpcScreenStatusChanged(QString, int);
    void onLogServiceOutput(QString text);
    void onLogCoreOutput(QString text);
    void onRpcCloudOffline();
    void onRpcCloudOnline();
    void onRpcAuthLogout();
    void onRpcVersionCheck();

private:
    boost::asio::io_service m_io;
    WampClient m_wampClient;
    std::unique_ptr<std::thread> m_rpcThread;
    std::shared_ptr<ErrorView> m_errorView;
};
