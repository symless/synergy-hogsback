#ifndef COMMONDIRECTORYMANAGER_H
#define COMMONDIRECTORYMANAGER_H

#include <string>
#include <boost/filesystem.hpp>

class  DirectoryManager
{
public:
    static DirectoryManager* instance();

    virtual std::string userDir();
    virtual std::string systemAppDir() = 0;
    virtual std::string installedDir() = 0;
    virtual std::string profileDir() = 0;
    virtual std::string pathSeparator() = 0;

    virtual boost::filesystem::path
    crashDumpDir() {
        boost::filesystem::path path (profileDir());
        path /= "dumps";
        boost::system::error_code ec;
        if (!boost::filesystem::exists (path, ec)) {
            boost::filesystem::create_directories (path);
        }
        return path;
    }

protected:
    DirectoryManager() = default;
    ~DirectoryManager() = default;

private:
    static DirectoryManager* s_instances;
};

#endif // COMMONDIRECTORYMANAGER_H
