#ifndef SCREENMANAGER_H
#define SCREENMANAGER_H

#include "LibMacro.h"
#include "ScreenListModel.h"

#include <QQuickItem>
#include <QMap>

class MulticastManager;
class IScreenArrangement;
class ProcessManager;
class ScreenListSnapshotManager;
class AppConfig;

class LIB_SPEC ScreenManager : public QObject
{
    Q_OBJECT

public:
    ScreenManager();
    ~ScreenManager();

    Q_PROPERTY(ScreenListModel* screenListModel READ screenListModel WRITE setScreenModel)
    Q_PROPERTY(ProcessManager* processManager WRITE setProcessManager)
    Q_PROPERTY(int viewWidth WRITE setViewWidth)
    Q_PROPERTY(int viewHeight WRITE setViewHeight)

    Q_INVOKABLE int getModelIndex(int x, int y);
    Q_INVOKABLE void moveModel(int index, int offsetX, int offsetY);
    Q_INVOKABLE void updateConfigFile();
    Q_INVOKABLE bool removeScreen(QString name, bool notify = false);
    Q_INVOKABLE void onKeyPressed(const int key);
    Q_INVOKABLE bool addScreen(QString name);

    // TODO: remove these debug functions
    Q_INVOKABLE void printBoundingBoxInfo();

    ScreenListModel* screenListModel() const;
    void setScreenModel(ScreenListModel* screenListModel);
    void setProcessManager(ProcessManager* processManager);
    void setViewWidth(int w);
    void setViewHeight(int h);
    void saveSnapshot();

private:
    int processMode();
    void setupWaitTimer();
    void startCoreProcess();

private slots:
    void handleDefaultGroupMessage(MulticastMessage msg);
    void handleUniqueGroupMessage(MulticastMessage msg);
    void waitServerReplyTimeout();

private:
    ScreenListModel* m_screenListModel;
    ProcessManager* m_processManager;
    MulticastManager* m_multicastManager;
    IScreenArrangement* m_arrangementStrategy;
    QTimer* m_waitTimer;
    ScreenListSnapshotManager* m_screenListSnapshotManager;
    AppConfig* m_appConfig;
    QMap<int, bool> m_defaultServerReplies;
    int m_latestConfigSerial;
};

#endif // SCREENMANAGER_H
