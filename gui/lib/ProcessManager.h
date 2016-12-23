#ifndef PROCESSMANAGER_H
#define PROCESSMANAGER_H

#include "LibMacro.h"

#include <QQuickItem>
#include <QProcess>

class ScreenListModel;
class IpcClient;

class LIB_SPEC ProcessManager : public QQuickItem
{
	Q_OBJECT

public:
	ProcessManager();

	Q_PROPERTY(IpcClient* ipcClient READ ipcClient WRITE setIpcClient)

	Q_INVOKABLE void start();

	IpcClient* ipcClient() const;
	void setIpcClient(IpcClient* ipcClient);

	int processMode();
	void setProcessMode(int mode);
	bool active();
	void setActive(bool active);

	QString serverIp() const;
	void setServerIp(const QString& serverIp);

private slots:
	void exit(int exitCode, QProcess::ExitStatus);
	void logOutput();
	void logError();

private:
	void startProcessOnWin();
	void startProcessOnUnix();

private:
	IpcClient* m_ipcClient;
	QProcess* m_process;
	int m_processMode;
	bool m_active;
	QString m_serverIp;
};

#endif // PROCESSMANAGER_H
