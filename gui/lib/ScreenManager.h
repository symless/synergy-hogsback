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
    Q_INVOKABLE void onKeyPressed(const int key);
    void saveSnapshot();

    // TODO: remove these debug functions
    Q_INVOKABLE void printBoundingBoxInfo();

    ScreenListModel* screenListModel() const;
    void setScreenModel(ScreenListModel* screenListModel);
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
    ScreenListModel* m_screenListModel;
    ProcessManager* m_processManager;
    MulticastManager* m_multicastManager;
    IScreenArrangement* m_arrangementStrategy;
    QTimer* m_waitTimer;
    ScreenListSnapshotManager* m_screenListSnapshotManager;
    QMap<int, bool> m_defaultServerReplies;
    int m_latestConfigSerial;
};

#endif // SCREENMANAGER_H
