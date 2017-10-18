#pragma once

#include <string>
#include <vector>
#include <boost/optional.hpp>

/* adapted version of the original gui command line gen class */

extern const std::string kCoreProgram;

class ProcessCommand
{
public:
    std::vector<std::string> generate(bool serverMode) const;
    void setServerAddress(const std::string& serverAddress);
    void setLocalHostname(const std::string& localHostname);
    void setRunAsUid(const std::string& runAsUid);

private:
    std::string m_serverAddress = "";
    std::string m_localHostname = "";
    std::string m_runAsUid = "";
};

const std::string kCoreLogFile = "synergy-core.log";
const std::string kCoreDebugLevel = "DEBUG";
