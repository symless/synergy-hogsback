#include "DirectoryManager.h"

#include "CoreInterface.h"
#include "LogManager.h"

#include <QStandardPaths>
#include <QDir>
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
    QString profileDir = QStandardPaths::standardLocations(QStandardPaths::AppConfigLocation).first();

    // persist the directory
    QDir dir = profileDir;
    if (!dir.exists()) {
        dir.mkdir(dir.path());
    }

    return profileDir;
}

QString DirectoryManager::configFileDir()
{
    return profileDir();
}

QString DirectoryManager::installedDir()
{
    return QCoreApplication::applicationDirPath();
}
