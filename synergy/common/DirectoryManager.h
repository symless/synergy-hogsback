#ifndef COMMONDIRECTORYMANAGER_H
#define COMMONDIRECTORYMANAGER_H

#include <boost/filesystem.hpp>

class  DirectoryManager
{
public:
    static DirectoryManager* instance();
    virtual boost::filesystem::path userDir();
    virtual boost::filesystem::path crashDumpDir();
    virtual boost::filesystem::path systemAppDir() = 0;
    virtual boost::filesystem::path installedDir() = 0;
    virtual boost::filesystem::path profileDir() = 0;
    virtual boost::filesystem::path systemLogDir() = 0;
};

extern std::string g_programDir;

#endif // COMMONDIRECTORYMANAGER_H
