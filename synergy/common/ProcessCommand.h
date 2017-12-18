#pragma once

#include <string>
#include <vector>
#include <boost/optional.hpp>

/* adapted version of the original gui command line gen class */

extern const std::string kCoreProgram;

class ProcessCommand
{
public:
    virtual ~ProcessCommand() = default;

    virtual std::vector<std::string> generate(bool serverMode) const;
    void setLocalHostname(const std::string& localHostname);
    bool setRunAsUid(std::string runAsUid);
    bool setDisplay(std::string display);

private:
    std::string m_serverAddress;
    std::string m_localHostname;
    std::string m_runAsUid;
    std::string m_display;
};

const std::string kCoreLogFile = "synergy-core.log";
const std::string kCoreDebugLevel = "DEBUG";
