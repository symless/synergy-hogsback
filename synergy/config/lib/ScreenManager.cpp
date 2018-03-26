#include "ScreenManager.h"

#include <synergy/config/lib/ServiceProxy.h>
#include "CloudClient.h"
#include "LogManager.h"
#include "ScreenBBArrangement.h"
#include "ScreenListSnapshotManager.h"
#include "AppConfig.h"
#include "ProcessMode.h"
#include <synergy/config/lib/Hostname.h>

#include <QtNetwork>
#include <iostream>

ScreenManager::ScreenManager() :
    m_screenListModel(NULL),
    m_screenListSnapshotManager(NULL)
{
    m_appConfig = qobject_cast<AppConfig*>(AppConfig::instance());

    m_arrangementStrategy = new ScreenBBArrangement();
    m_screenListSnapshotManager= new ScreenListSnapshotManager();

    m_cloudClient = qobject_cast<CloudClient*>(CloudClient::instance());

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
    if (!offsetX && !offsetY || index == -1) {
        return;
    }

    m_screenListModel->moveModel(index, offsetX, offsetY);
    m_arrangementStrategy->adjustModel(m_screenListModel, index);
    emit updateProfileConfig();
}

void ScreenManager::printBoundingBoxInfo()
{
    m_arrangementStrategy->printDebugInfo();
}

ScreenListModel* ScreenManager::screenListModel() const
{
    return m_screenListModel;
}

void ScreenManager::setScreenModel(ScreenListModel* screenListModel)
{
    if (m_screenListModel != screenListModel) {
        m_screenListModel = screenListModel;

        // this is for always showing local machine even without receiving snapshot
        addScreen(Hostname::local());
    }
}

void ScreenManager::setServiceProxy(ServiceProxy* serviceProxy)
{
    // weirdly, this function gets called on exit. when this happens
    // serviceProxy is null, so don't do anything in this case.
    if (serviceProxy == nullptr) {
        return;
    }

    m_serviceProxy = serviceProxy;

    connect(m_serviceProxy, SIGNAL(receivedScreens(QByteArray)),
            this, SLOT(updateScreens(QByteArray)));
    connect(m_serviceProxy, &ServiceProxy::screenError,
            this, &ScreenManager::onScreenError, Qt::QueuedConnection);

    m_serviceProxy->requestProfileSnapshot();
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
            LogManager::warning(QString("F12 no longer claims server, right click a screen instead"));
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

    UIScreen screen(name);
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

void ScreenManager::serverClaim(int index)
{
    const UIScreen& s = m_screenListModel->getScreen(index);

    m_serviceProxy->serverClaim(s.id());

    if (s.status() != ScreenStatus::kConnected) {
        LogManager::warning("sharing from computer that is not connected");
    }
}

bool ScreenManager::removeScreenById(int Id, bool notify)
{
    int index = m_screenListModel->findScreen(Id);

    return removeScreenByIndex(index, notify);
}

bool ScreenManager::removeScreenByIndex(int index, bool notify)
{
    if (index == -1) {
        LogManager::debug(QString("screen doesn't exist: %1")
                    .arg(index));
        return false;
    }


    const UIScreen& s = m_screenListModel->getScreen(index);
    bool result = false;

    if (s.id() != -1) {
        if (notify) {
            // notify cloud
            m_cloudClient->unsubProfile(s.id(), m_configVersion);
            m_configVersion++;
        }

        result = m_arrangementStrategy->removeScreen(m_screenListModel, s.id());
    }

    return result;
}

void ScreenManager::updateScreens(QByteArray reply)
{
    bool updateLocalHost = false;

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
            if (m_serverId != profileObject["server"].toInt()) {
                m_serverId = profileObject["server"].toInt();
                serverIdChanged();
            }

            QJsonArray screens = obj["screens"].toArray();
            QList<UIScreen> latestScreenList;
            QSet<int> latestScreenIdSet;
            int localScreenId = m_appConfig->screenId();
            foreach (QJsonValue const& v, screens) {
                QJsonObject obj = v.toObject();
                int screenId = obj["id"].toInt();
                latestScreenIdSet.insert(screenId);

                QString screenName = obj["name"].toString();
                int screenVersion = obj["version"].toInt();

                int index = m_screenListModel->findScreen(screenId);
                if (index != -1) {
                    const UIScreen& s = m_screenListModel->getScreen(index);

                    if (s.locked()) {
                        continue;
                    }

                    if (s.version() > screenVersion) {
                        LogManager::debug(QString("skip stale screen %1: %2 > %3").arg(screenId).arg(s.version()).arg(screenVersion));
                        continue;
                    }
                }

                UIScreen screen(screenName);
                screen.setId(screenId);
                screen.setPosX(obj["x_pos"].toInt());
                screen.setPosY(obj["y_pos"].toInt());
                screen.setStatus(obj["status"].toString());
                screen.setErrorCode(static_cast<ScreenError>(obj["error_code"].toInt()));
                screen.setErrorMessage(obj["error_message"].toString());
                if (!obj["active"].toBool()) {
                    screen.setStatus(ScreenStatus::kInactive);
                }
                screen.setVersion(screenVersion);

                latestScreenList.push_back(screen);
            }

            if (!m_screenIdSet.contains(localScreenId) &&
                !latestScreenIdSet.contains(localScreenId) &&
                m_appConfig->profileId() != -1) {
                latestScreenIdSet.insert(localScreenId);
                updateLocalHost = true;
            }

            // remove unsub screen
            m_screenIdSet.subtract(latestScreenIdSet);
            for (const int id : m_screenIdSet) {
                removeScreenById(id);

                if (id == localScreenId) {
                    emit localhostUnsubscribed();
                }
            }

            m_screenIdSet = latestScreenIdSet;

            m_screenListModel->update(latestScreenList);
        }
    }

    if (updateLocalHost) {
        removeScreenById(m_appConfig->screenId());
        UIScreen screen(Hostname::local());
        screen.setId(m_appConfig->screenId());
        m_arrangementStrategy->addScreen(m_screenListModel, screen);

        emit updateProfileConfig();
    }

    if (m_screenListModel->getScreenList().count() == 1) {
        setConfigHint("Next step: install Synergy 2 on another computer");
    }
    else {
        // clear the hint now we have more than 1 screen
        setConfigHint("");
    }
}

void ScreenManager::onUpdateProfileConfig()
{
    QJsonObject profileObject;
    profileObject.insert ("id", m_appConfig->profileId());
    profileObject.insert ("name", "default");

    QJsonArray screenArray;

    for (UIScreen& s : m_screenListModel->getScreenList()) {
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

void ScreenManager::onScreenError(QString screenName, int errorCode)
{
    // TODO: reimplement
}

QString ScreenManager::configHint()
{
    return m_configHint;
}

void ScreenManager::setConfigHint(const QString& text)
{
    m_configHint = text;
    configHintChanged();
}

void ScreenManager::restartServices()
{
    LogManager::debug("Resart services request");
    m_serviceProxy->restartService();
}

bool ScreenManager::isLocalMachine(int index)
{
    return index == m_screenListModel->findScreen(Hostname::local());
}
