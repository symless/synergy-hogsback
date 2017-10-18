#include <synergy/common/ProcessCommand.h>

#include <synergy/common/DirectoryManager.h>
#include <synergy/common/ConfigGen.h>

#include <boost/filesystem.hpp>

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

#ifdef __linux__
    // for use on linux, tell the core process what user id it should run as.
    // this is a simple way to allow the core process to talk to X. this avoids
    // the "WARNING: primary screen unavailable: unable to open screen" error.
    // a better way would be to use xauth cookie and dbus to get access to X.
    if (!m_runAsUid.empty()) {
        args.push_back("--run-as-uid");
        args.push_back(m_runAsUid);
    }
#endif

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

    auto logPath = DirectoryManager::instance()->systemLogDir() / kCoreLogFile;
    args.push_back("--log");
    args.push_back(logPath.string());

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

        args.push_back(m_serverAddress);
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

void ProcessCommand::setRunAsUid(const std::string& runAsUid)
{
    m_runAsUid = runAsUid;
}
