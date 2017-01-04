#include "ProcessCommand.h"

#include "AppConfig.h"
#include "DeviceManager.h"
#include "DirectoryManager.h"
#include "Common.h"

#include <QHostInfo>
#include <QMetaEnum>

const QString kServerCmd = "synergys.exe";
const QString kClientCmd = "synergyc.exe";

ProcessCommand::ProcessCommand(QObject* parent) :
    QObject(parent),
    m_deviceManager(NULL),
    m_directoryManager(NULL)
{

}

QString ProcessCommand::command(bool serverMode) const
{
    DirectoryManager directoryManager;
    QString command(directoryManager.installedDir());
    if (serverMode) {
        command += kServerCmd;
    }
    else {
        command += kClientCmd;
    }

    return wrapCommand(command);
}

QStringList ProcessCommand::arguments(bool serverMode) const
{
    DirectoryManager* directoryManager = NULL;
    if (m_directoryManager != NULL) {
        directoryManager = m_directoryManager;
    }
    else {
        directoryManager = new DirectoryManager();
    }

    AppConfig* appConfig =
            qobject_cast<AppConfig*>(AppConfig::instance());

    QStringList arguments;
    arguments << "-f" << "--no-tray" << "--ipc";

#ifdef Q_OS_WIN
    arguments << "--stop-on-desk-switch";
#endif

    // debug level
    arguments << "--debug";
    DebugLevel level = appConfig->debugLevel();
    QString debugLevelStr = debugLevelToStr(level);
    debugLevelStr = debugLevelStr.toUpper();
    arguments << debugLevelStr;

    // host name
    arguments << "--name";
    arguments << QHostInfo::localHostName();

#ifdef Q_OS_WIN
    // drag and drop
    if (appConfig->dragAndDrop()) {
        arguments << "--enable-drag-drop";
    }
#endif

    // profile directory
    arguments << "--profile-dir";
    arguments << wrapCommand(directoryManager->profileDir());

    if (serverMode) {
        // configuration file
        arguments << "-c";
        QString configFilename = directoryManager->configFileDir();
        configFilename += kDefaultConfigFile;
        arguments << configFilename;

#ifdef Q_OS_WIN
        // screen resolution info for DPI calculation, Windows only
        DeviceManager* deviceManager;
        if (m_deviceManager != NULL) {
            deviceManager = m_deviceManager;
        }
        else {
            deviceManager = new DeviceManager();
        }

        arguments << "--res-w";
        arguments << QString::number(deviceManager->resolutionWidth());
        arguments << "--res-h";
        arguments << QString::number(deviceManager->resolutionHeight());
        arguments << "--prm-wc";
        arguments << QString::number(deviceManager->primaryMonitorWidth() / 2);
        arguments << "--prm-hc";
        arguments << QString::number(deviceManager->primaryMonitorHeight() / 2);

        if (m_deviceManager == NULL) {
            delete deviceManager;
        }
#endif

        arguments << "--address";
        arguments << ":24800";
    }
    else {
        if (!m_serverIp.isEmpty()) {
            arguments << m_serverIp + ":24800";
        }
        else {
            // no server ip specified
            arguments.clear();
        }
    }

    arguments << "--log";
    arguments << "synergy.log";

    if (m_directoryManager == NULL) {
        delete m_directoryManager;
    }

    return arguments;
}

void ProcessCommand::setServerIp(const QString ip)
{
    m_serverIp = ip;
}

void ProcessCommand::setDeviceManager(DeviceManager* dm)
{
    m_deviceManager = dm;
}

void ProcessCommand::setDirectoryManager(DirectoryManager* dm)
{
    m_directoryManager = dm;
}

QString ProcessCommand::wrapCommand(QString command) const
{
    return QString("\"%1\"").arg(command);
}
