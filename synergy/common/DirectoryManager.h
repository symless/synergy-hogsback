#ifndef COMMONDIRECTORYMANAGER_H
#define COMMONDIRECTORYMANAGER_H

#include <string>

class  DirectoryManager
{
public:
    static DirectoryManager* instance();

    virtual std::string systemAppDir() = 0;

protected:
    DirectoryManager();
     ~DirectoryManager();

private:
    static DirectoryManager* s_instances;
};

#endif // COMMONDIRECTORYMANAGER_H
