#ifndef COMMONDIRECTORYMANAGER_H
#define COMMONDIRECTORYMANAGER_H

#include <boost/filesystem.hpp>

class  DirectoryManager
{
public:
    static DirectoryManager* instance();
    void init(const std::string& argv0);
    virtual boost::filesystem::path crashDumpDir();
    virtual boost::filesystem::path installDir() = 0;
    virtual boost::filesystem::path systemAppDir() = 0;
    virtual boost::filesystem::path profileDir() = 0;
    virtual boost::filesystem::path systemLogDir() = 0;

protected:
    virtual boost::filesystem::path userDir();

private:
    std::string m_argv0 = "";
};



#endif // COMMONDIRECTORYMANAGER_H
