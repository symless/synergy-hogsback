#include <synergy/common/ProcessCommand.h>

#include <synergy/common/DirectoryManager.h>
#include <synergy/common/ConfigGen.h>

#include <boost/filesystem.hpp>

const std::string kCoreLogFile = "synergy-core.log";
const std::string kCoreDebugLevel = "DEBUG";

#ifdef _WIN32
const std::string kCoreProgram = "synergy-core.exe";
#else
const std::string kCoreProgram = "synergy-core";
#endif

std::vector<std::string>
ProcessCommand::generate(bool serverMode) const
{
    auto profileDir = DirectoryManager::instance()->profileDir();
    auto installDir = DirectoryManager::instance()->installDir();

    std::vector<std::string> args;

    auto programPath = installDir / kCoreProgram;
    args.push_back(programPath.string());

    if (serverMode) {
        args.push_back("--server");
    }
    else {
        args.push_back("--client");
    }

    args.push_back("-f");

    args.push_back("--debug");
    args.push_back(kCoreDebugLevel);

    if (m_localHostname.empty()) {
        throw std::runtime_error("Can't generate args, local hostname missing.");
    }

    args.push_back("--name");
    args.push_back(m_localHostname);

    // TODO: change features depending on edition
    args.push_back("--enable-drag-drop");

    args.push_back("--profile-dir");
    args.push_back(profileDir.string());

    // TODO: use constant
    args.push_back("--log");
    args.push_back(kCoreLogFile);

    if (serverMode) {
        // configuration file
        auto configPath = profileDir / kCoreConfigFile;
        args.push_back("-c");
        args.push_back(configPath.string());

        args.push_back("--address");
        args.push_back(":24800");
    }
    else {
        if (m_serverAddress.empty()) {
            throw std::runtime_error("Can't generate args, server IP/hostname missing.");
        }

        args.push_back(m_serverAddress + ":24800");
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
