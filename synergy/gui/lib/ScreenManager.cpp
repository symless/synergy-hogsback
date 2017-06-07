#include "ScreenManager.h"

#include "CloudClient.h"
#include "ProcessManager.h"
#include "LogManager.h"
#include "ScreenBBArrangement.h"
#include "ConfigFileManager.h"
#include "ScreenListSnapshotManager.h"
#include "AppConfig.h"
#include "ProcessMode.h"

#include <QtNetwork>
#include <iostream>

ScreenManager::ScreenManager() :
    m_screenListModel(NULL),
    m_screenListSnapshotManager(NULL),
    m_latestConfigSerial(0),
    m_configVersion(-1),
    m_previousServerId(-1)
{
    m_appConfig = qobject_cast<AppConfig*>(AppConfig::instance());
    m_localHostname = QHostInfo::localHostName();

    m_arrangementStrategy = new ScreenBBArrangement();
    m_screenListSnapshotManager= new ScreenListSnapshotManager();

    m_cloudClient = qobject_cast<CloudClient*>(CloudClient::instance());

    connect(m_cloudClient, SIGNAL(receivedScreens(QByteArray)), this,
            SLOT(updateScreens(QByteArray)));

    connect(this, &ScreenManager::updateProfileConfig, this,
        &ScreenManager::onUpdateProfileConfig);
}

ScreenManager::~ScreenManager()
{
    delete m_arrangementStrategy;
    delete m_screenListSnapshotManager;
}

int ScreenManager::getModelIndex(int x, int y)
{
    return m_screenListModel->getModelIndex(x, y);
}

void ScreenManager::moveModel(int index, int offsetX, int offsetY)
{
    if (!offsetX && !offsetY) {
        return;
    }

    m_screenListModel->moveModel(index, offsetX, offsetY);
    m_arrangementStrategy->adjustModel(m_screenListModel, index);
    emit updateProfileConfig();

    if (processMode() == kServerMode) {
        startCoreProcess();
    }
}

void ScreenManager::updateConfigFile()
{
    ConfigFileManager configFileManager(m_screenListModel,
                            m_arrangementStrategy);
    configFileManager.writeConfigurationFile();
}

void ScreenManager::printBoundingBoxInfo()
{
    m_arrangementStrategy->printDebugInfo();

    LogManager::debug(QString("latest config serial: %1").arg(m_latestConfigSerial));
}

ScreenListModel* ScreenManager::screenListModel() const
{
    return m_screenListModel;
}

void ScreenManager::setScreenModel(ScreenListModel* screenListModel)
{
    if (m_screenListModel != screenListModel) {
        m_screenListModel = screenListModel;

        addScreen(QHostInfo::localHostName());
    }
}

void ScreenManager::setProcessManager(ProcessManager* processManager)
{
    m_processManager = processManager;

    connect(this, &ScreenManager::newServer, m_processManager,
        &ProcessManager::newServerDetected);
    connect(m_processManager, &ProcessManager::screenStatusChanged, this,
        &ScreenManager::onScreenStatusChanged);
    connect(m_processManager, &ProcessManager::localInputDetected, this,
            &ScreenManager::onLocalInputDetected);
}

void ScreenManager::setViewWidth(int w)
{
    if (w <= 0) return;

    m_arrangementStrategy->setViewW(w);
    m_arrangementStrategy->checkAdjustment(m_screenListModel, true);
}

void ScreenManager::setViewHeight(int h)
{
    if (h <= 0) return;

    m_arrangementStrategy->setViewH(h);
    m_arrangementStrategy->checkAdjustment(m_screenListModel, true);
}

void ScreenManager::saveSnapshot()
{
    m_screenListSnapshotManager->saveSnapshot(m_screenListModel);
}

void ScreenManager::onKeyPressed (int const key)
{
    switch (key) {
        case Qt::Key_F12:
            LogManager::debug(QString("claim to be server"));
            m_cloudClient->claimServer();
            //startCoreProcess();
            break;
    }
}

bool ScreenManager::addScreen(QString name)
{
    if (m_screenListModel->findScreen(name) != -1) {
        LogManager::debug(QString("screen already exist: %1")
                    .arg(name));
        return false;
    }

    Screen screen(name);
    return m_arrangementStrategy->addScreen(m_screenListModel, screen);
}

void ScreenManager::lockScreen(int index)
{
    m_screenListModel->lockScreen(index);
}

void ScreenManager::unlockScreen(int index)
{
    m_screenListModel->unlockScreen(index);
}

bool ScreenManager::removeScreen(QString name, bool notify)
{
    int index = m_screenListModel->findScreen(name);
    if (index == -1) {
        LogManager::debug(QString("screen doesn't exist: %1")
                    .arg(name));
        return false;
    }

    if (notify) {
        // get screen id
        const Screen& s = m_screenListModel->getScreen(index);

        // notify cloud
        if (s.id() != -1) {
            m_cloudClient->unsubProfile(s.id());
        }
    }

    Screen screen(name);
    bool result = m_arrangementStrategy->removeScreen(m_screenListModel,
                                            screen);

    return result;
}

int ScreenManager::processMode()
{
    if (m_processManager == NULL) {
        return kUnknownMode;
    }

    return m_processManager->processMode();
}

void ScreenManager::startCoreProcess()
{
    updateConfigFile();
    m_processManager->start();
}

void ScreenManager::updateScreens(QByteArray reply)
{
    bool updateLocalHost = false;
    bool newServerDetected = false;
    int serverId = m_previousServerId;

    QJsonDocument doc = QJsonDocument::fromJson(reply);
    if (!doc.isNull()) {
        if (doc.isObject()) {
            QJsonObject obj = doc.object();
            auto const& profileObject = obj["profile"].toObject();

            int const configVersion = profileObject["configVersion"].toInt();
            if (m_configVersion > configVersion) {
                return;
            }
            m_configVersion = configVersion;
            serverId = profileObject["server"].toInt();
            if (m_previousServerId != serverId) {
                // delay emitting the signal until we update all the screens
                newServerDetected = true;
            }

            QJsonArray screens = obj["screens"].toArray();
            QList<Screen> latestScreenList;
            QSet<QString> latestScreenNameSet;
            foreach (QJsonValue const& v, screens) {
                QJsonObject obj = v.toObject();
                QString screenName = obj["name"].toString();
                latestScreenNameSet.insert(screenName);
                int index = m_screenListModel->findScreen(screenName);
                if (index != -1) {
                    const Screen& s = m_screenListModel->getScreen(index);
                    if (s.locked()) {
                        continue;
                    }
                }

                Screen screen(screenName);
                screen.setId(obj["id"].toInt());
                screen.setPosX(obj["x_pos"].toInt());
                screen.setPosY(obj["y_pos"].toInt());
                screen.setStatus(obj["status"].toString());
                if (!obj["active"].toBool()) {
                    screen.setStatus(kInactive);
                }
                latestScreenList.push_back(screen);
            }

            if (!latestScreenNameSet.contains(m_localHostname) &&
                m_appConfig->profileId() != -1) {
                latestScreenNameSet.insert(m_localHostname);
                updateLocalHost = true;
            }

            // remove unsub screen
            m_screenNameSet.subtract(latestScreenNameSet);
            for (const QString& name : m_screenNameSet) {
                removeScreen(name);

                if (name == m_localHostname) {
                    emit localhostUnsubscribed();
                }
            }

            m_screenNameSet = latestScreenNameSet;

            m_screenListModel->update(latestScreenList);
        }
    }

    if (updateLocalHost) {
        removeScreen(m_localHostname);
        Screen screen(m_localHostname);
        screen.setId(m_appConfig->screenId());
        m_arrangementStrategy->addScreen(m_screenListModel, screen);

        emit updateProfileConfig();
    }

    if (newServerDetected) {
        updateConfigFile();
        emit newServer(serverId);
        m_previousServerId = serverId;

        QPair<QString, ScreenStatus> p (m_localHostname, kConnecting);
        onScreenStatusChanged(p);
    }
}

void ScreenManager::onUpdateProfileConfig()
{
    QJsonObject profileObject;
    profileObject.insert ("id", m_appConfig->profileId());
    profileObject.insert ("name", "default");

    QJsonArray screenArray;
    ;
    for (Screen& s : m_screenListModel->getScreenList()) {
        QJsonObject screenObject;
        screenObject.insert("id", s.id());
        screenObject.insert("x_pos", s.posX());
        screenObject.insert("y_pos", s.posY());

        screenArray.push_back(screenObject);
    }

    QJsonObject jsonObject;
    jsonObject.insert("profile", profileObject);
    jsonObject.insert("version", m_configVersion);
    jsonObject.insert("screens", screenArray);
    QJsonDocument doc(jsonObject);

    m_cloudClient->updateProfileConfig(doc);
    ++m_configVersion;
}

void ScreenManager::onScreenStatusChanged(QPair<QString, ScreenStatus> r)
{
    int index = m_screenListModel->findScreen(r.first);
    if (index != -1) {
        m_screenListModel->setScreenStatus(index, r.second);
        const Screen& s = m_screenListModel->getScreen(index);

        m_cloudClient->updateScreen(s);
    }
}

void ScreenManager::onLocalInputDetected()
{
    m_cloudClient->claimServer();
}
