#include "DirectoryManager.h"

#include "CoreInterface.h"
#include "LogManager.h"

#include <QStandardPaths>
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
    return QStandardPaths::standardLocations(QStandardPaths::AppConfigLocation).first();
}

QString DirectoryManager::configFileDir()
{
    return profileDir();
}

QString DirectoryManager::installedDir()
{
    return QCoreApplication::applicationDirPath();
}
