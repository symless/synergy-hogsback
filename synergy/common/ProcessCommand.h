#pragma once

#include <string>
#include <vector>

/* adapted version of the original gui command line gen class */

extern const std::string kServerCmd;
extern const std::string kClientCmd;

class ProcessCommand
{
public:
    std::vector<std::string> generate(bool serverMode) const;
    void setServerAddress(const std::string& serverAddress);
    void setLocalHostname(const std::string& localHostname);

private:
    std::string m_serverAddress;
    std::string m_localHostname;
};
