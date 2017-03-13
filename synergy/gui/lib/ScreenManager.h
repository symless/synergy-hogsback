#ifndef SCREENMANAGER_H
#define SCREENMANAGER_H

#include "LibMacro.h"
#include "ScreenListModel.h"

#include <QQuickItem>
#include <QMap>

class IScreenArrangement;
class ProcessManager;
class ScreenListSnapshotManager;
class AppConfig;
class CloudClient;

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
    Q_PROPERTY(CloudClient* cloudClient WRITE setCloudClient)

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
    void setCloudClient(CloudClient* cloudClient);
    void saveSnapshot();

signals:
    void updateGroupConfig();

private:
    int processMode();
    void startCoreProcess();

private slots:
    void updateScreens(QByteArray reply);
    void onUpdateGroupConfig();

private:
    ScreenListModel* m_screenListModel;
    ProcessManager* m_processManager;
    IScreenArrangement* m_arrangementStrategy;
    ScreenListSnapshotManager* m_screenListSnapshotManager;
    AppConfig* m_appConfig;
    CloudClient* m_cloudClient;
    QMap<int, bool> m_defaultServerReplies;
    QSet<QString> m_screenNameSet;
    QString m_localHostname;
    int m_latestConfigSerial;
};

#endif // SCREENMANAGER_H
