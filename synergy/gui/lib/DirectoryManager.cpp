#include "DirectoryManager.h"

#include "CoreInterface.h"
#include "LogManager.h"

#include <QStandardPaths>
#include <QDir>
#include <QCoreApplication>
#include <stdexcept>

GUIDirectoryManager::GUIDirectoryManager()
{

}

GUIDirectoryManager::~GUIDirectoryManager()
{

}

QString
GUIDirectoryManager::profileDir() const
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
GUIDirectoryManager::crashDumpDir() const
{
    QDir dir (profileDir() + "/dumps/");
    if (!dir.exists()) {
        dir.mkpath (dir.path());
    }
    return dir.path();
}

QString
GUIDirectoryManager::configFileDir() const
{
    return profileDir();
}

QString
GUIDirectoryManager::installedDir() const
{
    return QCoreApplication::applicationDirPath();
}
