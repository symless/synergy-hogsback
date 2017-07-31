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

QString
DirectoryManager::profileDir() const
{
    QString profileDir = QStandardPaths::standardLocations
                         (QStandardPaths::AppConfigLocation).first();
    QDir dir = profileDir;
    if (!dir.exists()) {
        dir.mkpath(dir.path());
    }

    return profileDir;
}

QString
DirectoryManager::crashDumpDir() const
{
    QDir dir(profileDir() + "/dumps/");
    if (!dir.exists()) {
        dir.mkpath (dir.path());
    }
    return dir.path();
}

QString
DirectoryManager::configFileDir() const
{
    return profileDir();
}

QString
DirectoryManager::installedDir() const
{
    return QCoreApplication::applicationDirPath();
}
