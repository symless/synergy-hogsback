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

signals:
    void screenStatusChanged(QPair<QString, ScreenStatus>);
    void rpcScreenStatusChanged(QString, int);
    void screenError(QString, int);
    void logCoreOutput(QString);
    void logServiceOutput(QString);
    void receivedScreens(QByteArray reply);

public slots:
    void onRpcScreenStatusChanged(QString, int);
    void onLogServiceOutput(QString text);
    void onLogCoreOutput(QString text);

private:
    boost::asio::io_service m_io;
    WampClient m_wampClient;
    std::unique_ptr<std::thread> m_rpcThread;
    std::shared_ptr<ErrorView> m_errorView;
    boost::asio::deadline_timer m_demoTimer;
};
