#include "ProcessManager.h"

#include "ProcessCommand.h"
#include "ScreenModel.h"
#include "IpcClient.h"
#include "LogManager.h"
#include "ProcessMode.h"

ProcessManager::ProcessManager() :
	m_ipcClient(NULL),
	m_process(NULL),
	m_processMode(kClientMode),
	m_active(true),
	m_serverIp()
{

}

IpcClient* ProcessManager::ipcClient() const
{
	return m_ipcClient;
}

void ProcessManager::setIpcClient(IpcClient *ipcClient)
{
	if (m_ipcClient != ipcClient) {
		m_ipcClient = ipcClient;
	}
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

void ProcessManager::start()
{
	if (m_processMode == kUnknownMode) {
		return;
	}

#ifdef Q_OS_WIN
	startProcessOnWin();
#else
	startProcessOnUnix();
#endif
}

void ProcessManager::startProcessOnWin()
{
	// service mode on Windows
	ProcessCommand processCommand;
	QString command = processCommand.command(m_processMode == kServerMode);
	if (m_processMode == kClientMode) {
		processCommand.setServerIp(m_serverIp);
	}
	QStringList args = processCommand.arguments(m_processMode == kServerMode);
	QString fullCommand(command + " " + args.join(" "));

	if (!args.empty()) {
		m_ipcClient->sendCommand(fullCommand, false);
	}
	else {
		LogManager::error(QString("invalid command: %1")
						  .arg(fullCommand));
	}
}

void ProcessManager::startProcessOnUnix()
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
	else {
		m_process = new QProcess(this);
		connect(m_process, SIGNAL(finished(int, QProcess::ExitStatus)),
			this, SLOT(exit(int, QProcess::ExitStatus)));
		connect(m_process, SIGNAL(readyReadStandardOutput()),
			this, SLOT(logOutput()));
		connect(m_process, SIGNAL(readyReadStandardError()),
			this, SLOT(logError()));

		ProcessCommand processCommand;
		QString command = processCommand.command(
								m_processMode == kServerMode);
		if (m_processMode == kClientMode) {
			processCommand.setServerIp(m_serverIp);
		}
		QStringList args = processCommand.arguments(
								m_processMode == kServerMode);

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
}

QString ProcessManager::serverIp() const
{
	return m_serverIp;
}

void ProcessManager::setServerIp(const QString& serverIp)
{
	m_serverIp = serverIp;
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

void ProcessManager::logOutput()
{
	if (m_process) {
		QString text(m_process->readAllStandardOutput());
		foreach(QString line, text.split(QRegExp("\r|\n|\r\n"))) {
			if (!line.isEmpty()) {
				LogManager::info(line);
			}
		}
	}
}

void ProcessManager::logError()
{
	if (m_process) {
		LogManager::error(m_process->readAllStandardError());
	}
}
