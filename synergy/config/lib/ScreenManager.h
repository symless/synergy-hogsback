#ifndef SCREENMANAGER_H
#define SCREENMANAGER_H

#include "LibMacro.h"
#include "ScreenListModel.h"

#include <QQuickItem>
#include <QMap>

class IScreenArrangement;
class ServiceProxy;
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
    Q_PROPERTY(ServiceProxy* serviceProxy WRITE setServiceProxy)
    Q_PROPERTY(int viewWidth WRITE setViewWidth)
    Q_PROPERTY(int viewHeight WRITE setViewHeight)
    Q_PROPERTY(QString configHint READ configHint NOTIFY configHintChanged)
    Q_PROPERTY(int server MEMBER m_serverId NOTIFY serverIdChanged)

    Q_INVOKABLE int getModelIndex(int x, int y);
    Q_INVOKABLE void moveModel(int index, int offsetX, int offsetY);
    Q_INVOKABLE bool removeScreen(QString name, bool notify = false);
    Q_INVOKABLE void onKeyPressed(const int key);
    Q_INVOKABLE bool addScreen(QString name);
    Q_INVOKABLE void lockScreen(int index);
    Q_INVOKABLE void unlockScreen(int index);
    Q_INVOKABLE void serverClaim(int index);

    // TODO: remove these debug functions
    Q_INVOKABLE void printBoundingBoxInfo();

    ScreenListModel* screenListModel() const;
    void setScreenModel(ScreenListModel* screenListModel);
    void setServiceProxy(ServiceProxy* serviceProxy);
    void setViewWidth(int w);
    void setViewHeight(int h);
    void saveSnapshot();
    QString configHint();

signals:
    void updateProfileConfig();
    void newServer(int serverId);
    void localhostUnsubscribed();
    void configHintChanged();
    void serverIdChanged();

private slots:
    void updateScreens(QByteArray reply);
    void onUpdateProfileConfig();
    void onScreenStatusChanged(QPair<QString, ScreenStatus> r);
    void onScreenError(QString screenName, int errorCode);

private:
    void setConfigHint(const QString &value);

private:
    ScreenListModel* m_screenListModel;
    ServiceProxy* m_serviceProxy = nullptr;
    IScreenArrangement* m_arrangementStrategy;
    ScreenListSnapshotManager* m_screenListSnapshotManager;
    AppConfig* m_appConfig;
    CloudClient* m_cloudClient;
    QSet<QString> m_screenNameSet;
    QMap<int, unsigned> m_screenVersionTracker;
    QString m_localHostname;
    int m_latestConfigSerial = 0;
    int m_configVersion = -1;
    int m_serverId = -1;
    QString m_configHint = "";
};

#endif // SCREENMANAGER_H
