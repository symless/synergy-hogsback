#include "MockDirectoryManager.h"

MockDirectoryManager::MockDirectoryManager()
{

}

QString MockDirectoryManager::profileDir()
{
    return "mockDir";
}

QString MockDirectoryManager::configFileDir()
{
    return "mockDir";
}

QString MockDirectoryManager::installDir()
{
    return "mockDir";
}

