#ifndef SCREENMANAGER_H
#define SCREENMANAGER_H

#include "LibMacro.h"
#include "ScreenModel.h"

#include <QQuickItem>
#include <QMap>

class MulticastManager;
class IScreenArrangement;
class ProcessManager;

class LIB_SPEC ScreenManager : public QObject
{
	Q_OBJECT

public:
	ScreenManager();
	~ScreenManager();

	Q_PROPERTY(ScreenModel* screenModel READ screenModel WRITE setScreenModel)
	Q_PROPERTY(ProcessManager* processManager WRITE setProcessManager)
	Q_PROPERTY(int viewWidth WRITE setViewWidth)
	Q_PROPERTY(int viewHeight WRITE setViewHeight)

	Q_INVOKABLE int getModelIndex(int x, int y);
	Q_INVOKABLE void moveModel(int index, int offsetX, int offsetY);
	Q_INVOKABLE void updateConfigFile();

	// TODO: remove these debug functions
	Q_INVOKABLE void printBoundingBoxInfo();

	ScreenModel* screenModel() const;
	void setScreenModel(ScreenModel* screenModel);
	void setProcessManager(ProcessManager* processManager);
	void setViewWidth(int w);
	void setViewHeight(int h);

private:
	bool addScreen(QString name);
	bool removeScreen(QString name);
	int processMode();
	void setupWaitTimer();

private slots:
	void handleDefaultGroupMessage(MulticastMessage msg);
	void handleUniqueGroupMessage(MulticastMessage msg);
	void waitServerReplyTimeout();

private:
	ScreenModel* m_screenModel;
	ProcessManager* m_processManager;
	MulticastManager* m_multicastManager;
	IScreenArrangement* m_arrangementStrategy;
	QTimer* m_waitTimer;
	QMap<int, bool> m_defaultServerReplies;
	QList<Screen> m_configScreensRecord;
	int m_latestConfigSerial;
};

#endif // SCREENMANAGER_H
