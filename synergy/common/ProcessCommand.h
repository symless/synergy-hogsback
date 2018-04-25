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

    virtual std::vector<std::string> serverCmd(const std::string localHostname) const;
    virtual std::vector<std::string> clientCmd(const std::string localHostname) const;
    bool setRunAsUid(std::string runAsUid);
    bool setDisplay(std::string display);

private:
    std::vector<std::string> generate(bool serverMode, const std::string localHostname) const;

private:
    std::string m_serverAddress;
    std::string m_runAsUid;
    std::string m_display;
};

const std::string kCoreLogFile = "synergy-core.log";
const std::string kCoreDebugLevel = "DEBUG";
