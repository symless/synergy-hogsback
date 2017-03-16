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
    return installedDir();
}

QString DirectoryManager::configFileDir()
{
    return installedDir();
}

QString DirectoryManager::installedDir()
{
    return QCoreApplication::applicationDirPath();
}
