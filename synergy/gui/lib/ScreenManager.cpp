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
    m_latestConfigSerial(0)
{
    m_appConfig = qobject_cast<AppConfig*>(AppConfig::instance());
    m_localHostname = QHostInfo::localHostName();
    m_arrangementStrategy = new ScreenBBArrangement();

    m_screenListSnapshotManager= new ScreenListSnapshotManager();

    connect(this, &ScreenManager::updateGroupConfig, this,
        &ScreenManager::onUpdateGroupConfig);
}

ScreenManager::~ScreenManager()
{
    delete m_arrangementStrategy;
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
    emit updateGroupConfig();

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

void ScreenManager::setCloudClient(CloudClient *cloudClient)
{
    if (cloudClient == NULL) return;

    m_cloudClient = cloudClient;
    connect(m_cloudClient, SIGNAL(receivedScreens(QByteArray)), this,
            SLOT(updateScreens(QByteArray)));
}

void ScreenManager::saveSnapshot()
{
    m_screenListSnapshotManager->saveSnapshot(m_screenListModel);
}

void ScreenManager::onKeyPressed (int const key)
{
    switch (key) {
        case Qt::Key_S:
            saveSnapshot();
            break;
        case Qt::Key_A:
            startCoreProcess();
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

bool ScreenManager::removeScreen(QString name, bool notify)
{
    if (m_screenListModel->findScreen(name) == -1) {
        LogManager::debug(QString("screen doesn't exist: %1")
                    .arg(name));
        return false;
    }

    Screen screen(name);
    bool result = m_arrangementStrategy->removeScreen(m_screenListModel,
                                            screen);

    // TODO: need to do unsubscribe from cloud client
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

    QJsonDocument doc = QJsonDocument::fromJson(reply);
    if (!doc.isNull()) {
        if (doc.isObject()) {
            QJsonObject obj = doc.object();

            QJsonArray screens = obj["screens"].toArray();
            QList<Screen> latestScreenList;
            QSet<QString> latestScreenNameSet;
            foreach (QJsonValue const& v, screens) {
                QJsonObject obj = v.toObject();
                QString screenName = obj["name"].toString();
                latestScreenNameSet.insert(screenName);
                Screen screen(screenName);
                screen.setId(obj["id"].toInt());
                screen.setPosX(obj["posX"].toInt());
                screen.setPosY(obj["posY"].toInt());
                screen.setState(kDisconnected);
                if (!obj.contains("activeGroup") ||
                    obj["expired"].toBool()) {
                    screen.setState(kInactive);
                }

                latestScreenList.push_back(screen);
            }

            if (!latestScreenNameSet.contains(m_localHostname)) {
                latestScreenNameSet.insert(m_localHostname);
                updateLocalHost = true;
            }
            // remove unsub screen
            m_screenNameSet.subtract(latestScreenNameSet);
            for (const QString& name : m_screenNameSet) {
                removeScreen(name);
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

        emit updateGroupConfig();
    }
}

void ScreenManager::onUpdateGroupConfig()
{
    QJsonObject groupObject;
    groupObject.insert ("id", m_appConfig->groupId());
    groupObject.insert ("name", "default");

    QJsonArray screenArray;
    ;
    for (Screen& s : m_screenListModel->getScreenList()) {
        QJsonObject screenObject;
        screenObject.insert("id", s.id());
        screenObject.insert("posX", s.posX());
        screenObject.insert("posY", s.posY());

        screenArray.push_back(screenObject);
    }

    QJsonObject jsonObject;
    jsonObject.insert("group", groupObject);
    jsonObject.insert("screenRenderInfo", screenArray);
    QJsonDocument doc(jsonObject);

    m_cloudClient->updateGroupConfig(doc);
}
