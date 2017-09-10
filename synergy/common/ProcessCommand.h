#pragma once

#include <string>
#include <vector>

/* adapted version of the original gui command line gen class */

class ProcessCommand
{
public:
    std::string print(bool serverMode) const;
    void setServerAddress(const std::string& serverAddress);
    void setLocalHostname(const std::string& localHostname);

private:
    std::string arguments(bool serverMode) const;
    std::string wrapArg(const std::string& arg) const;

private:
    std::string m_serverAddress;
    std::string m_localHostname;
};
