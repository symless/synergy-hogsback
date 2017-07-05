#ifndef COMMONGLOBALCONFIG_H
#define COMMONGLOBALCONFIG_H

#include <string>

class  GlobalConfig
{
public:
    GlobalConfig();
    std::string filename();
    void load();

private:
    std::string m_lastProfileFilename;
    std::string m_lastUserToken;
};

#endif // COMMONGLOBALCONFIG_H
