#include "DirectoryManager.h"

#include "CoreInterface.h"
#include "LogManager.h"

#include <QCoreApplication>
#include <stdexcept>

DirectoryManager::DirectoryManager()
{

}

DirectoryManager::~DirectoryManager()
{

}

QString DirectoryManager::profileDir()
{
	CoreInterface coreInterface;

	try {
		return coreInterface.profileDir();
	}
	catch (std::runtime_error& e) {
		LogManager::error(QString("failed to get profile directory, %1")
							.arg(e.what()));
		// TODO: remove this test code
		return "\"C:\\Users\\Jerry\\AppData\\Local\"";
		//throw e;
	}
}

QString DirectoryManager::configFileDir()
{
	return installedDir();
}

QString DirectoryManager::installedDir()
{
	// TODO: remove this test code
	return "C:\\Users\\Jerry\\Desktop\\";
	//return QCoreApplication::applicationDirPath();
}
