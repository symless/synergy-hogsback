#include <synergy/common/ProcessCommand.h>

#include <synergy/common/DirectoryManager.h>
#include <synergy/common/ConfigGen.h>

#include <boost/filesystem.hpp>

const std::string kCoreLogFile = "synergy.log";

#ifdef _WIN32
const std::string kServerCmd = "synergys.exe";
const std::string kClientCmd = "synergyc.exe";
#else
const std::string kServerCmd = "synergys";
const std::string kClientCmd = "synergyc";
#endif

std::string
ProcessCommand::wrapArg(const std::string& arg) const
{
    return "\"" + arg + "\"";
}

std::vector<std::string>
ProcessCommand::generate(bool serverMode) const
{
    auto profileDir = DirectoryManager::instance()->profileDir();
    auto installedDir = DirectoryManager::instance()->installedDir();

    std::vector<std::string> args;

    std::string processName = serverMode ? kServerCmd : kClientCmd;
    auto processPath = installedDir / processName;
    args.push_back(processPath.string());

    args.push_back("-f");
    args.push_back("--no-tray");

    // TODO: set debug level based on settings
    args.push_back("--debug");
    args.push_back("DEBUG");

    if (m_localHostname.empty()) {
        throw std::runtime_error("Can't generate args, local hostname missing.");
    }

    args.push_back("--name");
    args.push_back(wrapArg(m_localHostname));

    // TODO: change features depending on edition
    args.push_back("--enable-drag-drop");

    args.push_back("--profile-dir");
    args.push_back(wrapArg(profileDir.string()));

    // TODO: use constant
    args.push_back("--log");
    args.push_back(kCoreLogFile);

    if (serverMode) {
        // configuration file
        auto configPath = profileDir / kCoreConfigFile;
        args.push_back("-c");
        args.push_back(wrapArg(configPath.string()));

        args.push_back("--address");
        args.push_back(":24800");
    }
    else {
        if (m_serverAddress.empty()) {
            throw std::runtime_error("Can't generate args, server IP/hostname missing.");
        }

        args.push_back(wrapArg(m_serverAddress + ":24800"));
    }

    return args;
}

void
ProcessCommand::setServerAddress(const std::string& serverAddress)
{
    m_serverAddress = serverAddress;
}

void
ProcessCommand::setLocalHostname(const std::string& localHostname)
{
    m_localHostname = localHostname;
}
