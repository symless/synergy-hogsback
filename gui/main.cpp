#include "Hostname.h"
#include "ScreenModel.h"
#include "ScreenManager.h"
#include "IpcClient.h"
#include "LogManager.h"
#include "ProcessManager.h"
#include "Common.h"

#include <QApplication>
#include <QtQuick>
#include <QQmlApplicationEngine>
#include <stdexcept>

int main(int argc, char* argv[])
{
	QApplication app(argc, argv);
	LogManager::instance();

	try {
		qmlRegisterType<Hostname>("com.synergy.gui", 1, 0, "Hostname");
		qmlRegisterType<ScreenModel>("com.synergy.gui", 1, 0, "ScreenModel");
		qmlRegisterType<ScreenManager>("com.synergy.gui", 1, 0, "ScreenManager");
		qmlRegisterType<IpcClient>("com.synergy.gui", 1, 0, "IpcClient");
		qmlRegisterType<ProcessManager>("com.synergy.gui", 1, 0, "ProcessManager");
		QQmlApplicationEngine engine(QUrl(QStringLiteral("qrc:/main.qml")));

		return app.exec();
	}
	catch (std::runtime_error& e) {
		LogManager::error(QString("exception catched: ")
							.arg(e.what()));
		return 0;
	}
}
