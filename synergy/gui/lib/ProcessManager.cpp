#include "ProcessManager.h"

#include "ConnectivityTester.h"
#include "ProcessCommand.h"
#include "ScreenListModel.h"
#include "LogManager.h"
#include "ProcessMode.h"
#include "AppConfig.h"
#include <synergy/common/RpcClient.h>

#include <QtNetwork>
#include <QtGlobal>
#include <iostream>
#ifndef Q_OS_WIN
#include <unistd.h>
#endif

ProcessManager::ProcessManager(RpcClient& rpcClient) :
    m_process(NULL),
    m_processMode(kClientMode),
    m_active(true),
    m_serverIp(),
    m_ioService(rpcClient.getIoService()),
    m_rpcClient(rpcClient)
{
    m_appConfig = qobject_cast<AppConfig*>(AppConfig::instance());
}

ProcessManager::~ProcessManager()
{
}

int ProcessManager::processMode()
{
    return m_processMode;
}

void ProcessManager::setProcessMode(int mode)
{
    m_processMode = mode;
}

bool ProcessManager::active()
{
    return m_active;
}

void ProcessManager::setActive(bool active)
{
    m_active = active;
}

ConnectivityTester* ProcessManager::connectivityTester()
{
    return m_connectivityTester;
}

void ProcessManager::setConnectivityTester(ConnectivityTester* tester)
{
    m_connectivityTester = tester;
}

void ProcessManager::start()
{
    if (m_processMode == kUnknownMode) {
        return;
    }

    ProcessCommand processCommand;
    QString command = processCommand.command(
                            m_processMode == kServerMode);
    if (m_processMode == kClientMode) {
        processCommand.setServerIp(m_serverIp);
    }
    QStringList args = processCommand.arguments(
                            m_processMode == kServerMode);
    std::vector<std::string> cmd;
    cmd.push_back(command.toStdString());
    for (auto& arg :args) {
        cmd.push_back(arg.toStdString());
    }

    m_rpcClient.call<void> ("startCore", cmd);
    // startProcess();
}


void ProcessManager::startProcess()
{
    // desktop mode on Unix
    if (m_process != NULL) {
        // stop the previous process if exists
        if (m_process->isOpen()) {
            m_process->close();
        }

        delete m_process;
        m_process = NULL;
    }

    m_process = new QProcess(this);
    connect(m_process, SIGNAL(finished(int, QProcess::ExitStatus)),
        this, SLOT(exit(int, QProcess::ExitStatus)));
    connect(m_process, SIGNAL(readyReadStandardOutput()),
        this, SLOT(logCoreOutput()));
    connect(m_process, SIGNAL(readyReadStandardError()),
        this, SLOT(logCoreError()));

    ProcessCommand processCommand;
    QString command = processCommand.command(
                            m_processMode == kServerMode);
    if (m_processMode == kClientMode) {
        processCommand.setServerIp(m_serverIp);
    }
    QStringList args = processCommand.arguments(
                            m_processMode == kServerMode);
#ifndef Q_OS_WIN
    auto argstr = args.join(" ").toStdString();
    std::cout << getcwd(NULL, 0) << std::endl;
    std::cout << command.toStdString() << std::endl;
    std::cout << argstr << std::endl;
#endif
    m_process->start(command, args);

    if (!m_process->waitForStarted()) {
        LogManager::error(QString("Program can not be started, "
                            "The executable<br><br>%1<br><br>"
                            "could not be successfully started, "
                            "although it does exist. Please "
                            "check if you have sufficient "
                            "permissions to run this program.")
                            .arg(command));
        return;
    }
}

QString ProcessManager::serverIp() const
{
    return m_serverIp;
}

void ProcessManager::setServerIp(const QString& serverIp)
{
    m_serverIp = serverIp;
}

void ProcessManager::newServerDetected(int serverId)
{
    // decide which mode local screen should be
    if (serverId == m_appConfig->screenId()) {
        setProcessMode(kServerMode);
    }
    else {
        setProcessMode(kClientMode);

        QStringList r = m_connectivityTester->getSuccessfulResults(serverId);

        if (!r.empty()) {
            // TODO: furthur ip matching test
            setServerIp(r.first());
            LogManager::debug(QString("connecting to server: %1").arg(r.first()));
        }
        else {
            LogManager::debug(QString("can not find any successful connectivity result for the server screen: %1").arg(serverId));
            LogManager::debug(QString("retry in 3 seconds"));
            QTimer::singleShot(3000, this, [=](){
                newServerDetected(serverId);
            });
            return;
        }
    }

    start();
}

void ProcessManager::exit(int exitCode, QProcess::ExitStatus)
{
    if (exitCode == 0) {
        LogManager::info("process exited normally");
    }
    else {
        LogManager::error(QString("process exited with error code: %1")
                            .arg(exitCode));
    }
}

void ProcessManager::logCoreOutput()
{
    if (m_process) {
        QString text(m_process->readAllStandardOutput());
        foreach(QString line, text.split(QRegExp("\r|\n|\r\n"))) {
            if (!line.isEmpty()) {
                LogManager::raw("[Core] " + line);

                // TODO: use proper IPC
                // check key outputs
                if (line.contains("started server, waiting for clients") ||
                    line.contains("connected to server")) {
                    QPair<QString, ScreenStatus> r;
                    r.first = QHostInfo::localHostName();
                    r.second = kConnected;
                    emit screenStatusChanged(r);
                }
                else if (line.contains("\" has connected")) {
                    QPair<QString, ScreenStatus> r;
                    QStringList result = line.split('"');
                    Q_ASSERT(result.size() == 3);

                    if (result.size() == 3) {
                        r.first = result[1];
                        r.second = kConnected;
                        emit screenStatusChanged(r);
                    }
                }
                else if (line.contains("connecting to")) {
                    QPair<QString, ScreenStatus> r;
                    r.first = QHostInfo::localHostName();
                    r.second = kConnecting;
                    emit screenStatusChanged(r);
                } else if (line.contains("local input detected")) {
                    emit localInputDetected();
                }
            }
        }
    }
}

void ProcessManager::logCoreError()
{
    if (m_process) {
        LogManager::error(m_process->readAllStandardError());
    }
}
