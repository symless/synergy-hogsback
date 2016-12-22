#include "ScreenManager.h"

#include "MulticastManager.h"
#include "ProcessManager.h"
#include "ConfigMessageConvertor.h"
#include "LogManager.h"
#include "ScreenBBArrangement.h"
#include "ConfigFileManager.h"
#include "ProcessMode.h"

#include <QtNetwork>

ScreenManager::ScreenManager() :
	m_screenModel(NULL),
	m_multicastManager(NULL),
	m_waitTimer(NULL),
	m_latestConfigSerial(0)
{
	m_arrangementStrategy = new ScreenBBArrangement();

	m_multicastManager = qobject_cast<MulticastManager*>(
								MulticastManager::instance());
	connect(m_multicastManager,
		SIGNAL(receivedDefaultGroupMessage(MulticastMessage)),
		this, SLOT(handleDefaultGroupMessage(MulticastMessage)));
	connect(m_multicastManager,
		SIGNAL(receivedUniqueGroupMessage(MulticastMessage)),
			this, SLOT(handleUniqueGroupMessage(MulticastMessage)));

	setupWaitTimer();
	m_multicastManager->multicastDefaultExistence();

}

ScreenManager::~ScreenManager()
{
	delete m_arrangementStrategy;

	m_multicastManager->multicastUniqueLeave(true);
}

int ScreenManager::getModelIndex(int x, int y)
{
	return m_screenModel->getModelIndex(x, y);
}

void ScreenManager::moveModel(int index, int offsetX, int offsetY)
{
	m_screenModel->moveModel(index, offsetX, offsetY);
	m_arrangementStrategy->adjustModel(m_screenModel, index);

	if (m_multicastManager->joinedUniqueGroup()) {
		ConfigMessageConvertor convertor;
		Screen screen;
		screen = m_screenModel->getScreen(index);
		QString data = convertor.fromScreenToString(screen);
		m_multicastManager->multicastUniqueConfigDelta(data);
	}
}

void ScreenManager::updateConfigFile()
{
	ConfigFileManager configFileManager(m_screenModel,
							m_arrangementStrategy);
	configFileManager.writeConfigurationFile();
}

void ScreenManager::printBoundingBoxInfo()
{
	m_arrangementStrategy->printDebugInfo();

	LogManager::debug(QString("latest config serial: %1").arg(m_latestConfigSerial));
}

void ScreenManager::removeLast()
{
	Screen screen(m_lastAddedScreen);
	m_arrangementStrategy->removeScreen(m_screenModel, screen);
}

ScreenModel* ScreenManager::screenModel() const
{
	return m_screenModel;
}

void ScreenManager::setScreenModel(ScreenModel* screenModel)
{
	if (m_screenModel != screenModel) {
		m_screenModel = screenModel;

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
	m_arrangementStrategy->checkAdjustment(m_screenModel, true);
}

void ScreenManager::setViewHeight(int h)
{
	if (h <= 0) return;

	m_arrangementStrategy->setViewH(h);
	m_arrangementStrategy->checkAdjustment(m_screenModel, true);
}

bool ScreenManager::addScreen(QString name)
{
	if (m_screenModel->findScreen(name)	!= -1) {
		LogManager::debug(QString("screen already exist: %1")
					.arg(name));
		return false;
	}

	m_lastAddedScreen = name;
	Screen screen(name);
	return m_arrangementStrategy->addScreen(m_screenModel, screen);
}

bool ScreenManager::removeScreen(QString name)
{
	if (m_screenModel->findScreen(name)	== -1) {
		LogManager::debug(QString("screen doesn't exist: %1")
					.arg(name));
		return false;
	}

	Screen screen(name);
	return m_arrangementStrategy->removeScreen(m_screenModel, screen);
}

int ScreenManager::processMode()
{
	if (m_processManager == NULL) {
		return kUnknownMode;
	}

	return m_processManager->processMode();
}

void ScreenManager::setupWaitTimer()
{
	if (m_waitTimer == NULL) {
		m_waitTimer = new QTimer(this);
		m_waitTimer->setSingleShot(true);
		connect(m_waitTimer, SIGNAL(timeout()), this,
			SLOT(waitServerReplyTimeout()));
		m_waitTimer->start(3000);

		m_defaultServerReplies.clear();
	}
}

void ScreenManager::handleDefaultGroupMessage(MulticastMessage msg)
{
	// servers need to send replies on receiving default existence
	if (processMode() == kServerMode) {
		if (msg.m_type == MulticastMessage::kDefaultExistence) {
			if (m_multicastManager->joinedUniqueGroup()) {
				// multicast server reply
				bool active = false;
				if (m_processManager != NULL) {
					active = m_processManager->active();
				}
				m_multicastManager->multicastDefaultServerReply(active);
			}
		}
	}
	// clients either join active servers or switch to server if no server
	// exists
	else if (processMode() == kClientMode) {
		if (msg.m_type == MulticastMessage::kDefaultReply) {
			if (m_multicastManager->joinedUniqueGroup() == false) {
				setupWaitTimer();
				m_defaultServerReplies.insert(msg.m_uniqueGroup.toInt(),
											msg.m_active);
			}
		}
	}
	// to calculate next available unique group
	// TODO: probably move this to another place
	else if (processMode() == kUnknownMode) {

	}
}

void ScreenManager::handleUniqueGroupMessage(MulticastMessage msg)
{
	if (processMode() == kServerMode) {
		if (msg.m_type == MulticastMessage::kUniqueJoin) {
			if (addScreen(msg.m_hostname)) {
				LogManager::info(QString("client %1 joined server unique "
										 "group").arg(msg.m_hostname));

				// claim this is the server
				m_multicastManager->multicastUniqueClaim();

				// sync configuration information
				ConfigMessageConvertor convertor;
				QString data = convertor.fromModelToString(m_screenModel);
				m_multicastManager->multicastUniqueConfig(data,
										++m_latestConfigSerial);
			}
		}
		else if (msg.m_type == MulticastMessage::kUniqueLeave) {
			if (removeScreen(msg.m_hostname)) {
				LogManager::info(QString("client %1 left server unique "
										 "group").arg(msg.m_hostname));

				// only sync the changed part in configuration
				ConfigMessageConvertor convertor;
				Screen screen(msg.m_hostname);
				QString data = convertor.fromScreenToString(screen);
				m_multicastManager->multicastUniqueConfigDelta(data);
			}
		}
	}
	else if (processMode() == kClientMode) {
		if (msg.m_type == MulticastMessage::kUniqueLeave &&
			msg.m_ip == m_processManager->serverIp()) {
			m_processManager->setServerIp("");
			m_multicastManager->leaveUniqueGroup();
			m_multicastManager->joinDefaultGroup();
			setupWaitTimer();
			m_multicastManager->multicastDefaultExistence();
		}
	}

	if (msg.m_type == MulticastMessage::kUniqueClaim) {
		m_processManager->setProcessMode(kClientMode);
		m_processManager->setServerIp(msg.m_ip);

		//m_processManager->start();
	}
	else if (msg.m_type == MulticastMessage::kUniqueConfig ||
			 msg.m_type == MulticastMessage::kUniqueConfigDelta) {
		ConfigMessageConvertor convertor;
		if(convertor.fromStringToList(m_configScreensRecord,
						msg.m_configInfo,
						m_latestConfigSerial,
						msg.m_type == MulticastMessage::kUniqueConfig)) {

			// if there is only 1 screen in the list and its position is -1,-1,
			// it means remove this screen in configuration
			if (m_configScreensRecord.size() == 1) {
				if (m_configScreensRecord[0].posX() == -1 &&
					m_configScreensRecord[0].posY() == -1) {
					removeScreen(m_configScreensRecord[0].name());
					return;
				}
			}

			m_screenModel->update(m_configScreensRecord);
			m_arrangementStrategy->update(m_screenModel);
		}
	}
}

void ScreenManager::waitServerReplyTimeout()
{
	int group = m_multicastManager->getFirstActiveServerUniqueGroup(
							m_defaultServerReplies);

	if (group != -1) {
		m_multicastManager->joinUniqueGroup(group);
		m_multicastManager->multicastUniqueJoin(kClientMode);
		m_processManager->setProcessMode(kClientMode);
		m_multicastManager->leaveDefaultGroup();

		LogManager::debug(QString("join exist server"));
	}
	else {
		group = m_multicastManager->getNextAvailableUniqueGroup(
					m_defaultServerReplies);

		if (group == -1) {
			LogManager::error(QString("failed to find any active server"
									  "and next available group"));
			return;
		}

		// join unique group and switch to server
		m_multicastManager->joinUniqueGroup(group);
		m_processManager->setProcessMode(kServerMode);

		LogManager::debug(QString("switch to server"));
	}

	disconnect(m_waitTimer, SIGNAL(timeout()), this,
		SLOT(waitServerReplyTimeout()));
	delete m_waitTimer;
	m_waitTimer = NULL;
}
