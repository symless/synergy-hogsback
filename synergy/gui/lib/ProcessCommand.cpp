#include "ProcessCommand.h"

#include "AppConfig.h"
#include "DirectoryManager.h"
#include "Common.h"

#include <QHostInfo>
#include <QMetaEnum>

#ifdef Q_OS_WIN
const QString kServerCmd = "synergys.exe";
const QString kClientCmd = "synergyc.exe";
#else
const QString kServerCmd = "synergys";
const QString kClientCmd = "synergyc";
#endif

ProcessCommand::ProcessCommand(QObject* parent) :
    QObject(parent),
    m_directoryManager(NULL)
{

}

QString ProcessCommand::command(bool serverMode) const
{
    DirectoryManager directoryManager;
    QString command(directoryManager.installedDir());
    command += '/';

    if (serverMode) {
        command += kServerCmd;
    }
    else {
        command += kClientCmd;
    }

    return command;
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
    arguments << "-f" << "--no-tray";

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

    arguments << "--log";
    arguments << "synergy.log";

    if (serverMode) {
        // configuration file
        arguments << "-c";
        QString configFilename = directoryManager->configFileDir();
        configFilename += '/';
        configFilename += kDefaultConfigFile;
        arguments << configFilename;

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

    if (m_directoryManager == NULL) {
        delete m_directoryManager;
    }

    return arguments;
}

void ProcessCommand::setServerIp(const QString ip)
{
    m_serverIp = ip;
}

void ProcessCommand::setDirectoryManager(DirectoryManager* dm)
{
    m_directoryManager = dm;
}

QString ProcessCommand::wrapCommand(QString command) const
{
    return QString("\"%1\"").arg(command);
}
