#include <cstdlib>
#include <unistd.h>
#include <fstream>
#include <time.h>
#include <fmt/format.h>
#include <boost/filesystem.hpp>
#include <string>
#include <stdexcept>
#include <cassert>
#include <time.h>
#include <array>

std::string const kSharedConfigPath ("/Users/Shared/Synergy");
std::string const kAppPath ("/Applications/Synergy.app");
std::string const kAppResourcePath (kAppPath + "/Contents/Resources");

auto const kAppVersionFilePath = kAppResourcePath + "/Version.txt";

std::string const kHelperName = "com.symless.synergy.v2.ServiceHelper";
auto const kHelperPListPath = "/Library/LaunchDaemons/" + kHelperName + ".plist";
auto const kHelperExecPath = "/Library/PrivilegedHelperTools/" + kHelperName;

std::string const kServiceUserAgentFilename = "com.symless.synergy.v2.synergyd.plist";
auto const kServiceUserAgentPListTargetPath = "/Library/LaunchAgents/" + kServiceUserAgentFilename;
auto const kServiceUserAgentPListSourcePath = kAppResourcePath + "/" + kServiceUserAgentFilename;

static std::ofstream&
log() {
    static std::ofstream lofs(kSharedConfigPath + "/helper.log", std::ios::out | std::ios::app);
    assert (lofs.is_open());
    return lofs;
}

static
std::string timestamp() {
    time_t t;
    ::time (&t);
    std::array<char, 64> ct_buf;
    auto ct = ::ctime_r (&t, ct_buf.data());
    assert (ct == ct_buf.data());
    /* ctime_r() oddly terminates the string with a newline */
    std::replace (begin(ct_buf), end(ct_buf), '\n', '\0');
    return ct;
}

static bool
installSynergyService()
{
    if (boost::filesystem::exists (kServiceUserAgentPListTargetPath)) {
        log() << fmt::format ("[{}] service already installed, we're done\n", timestamp());
        return true;
    }
    if (!boost::filesystem::exists (kServiceUserAgentPListSourcePath)) {
        log() << fmt::format ("[{}] service plist file doesn't exist\n", timestamp());
        return false;
    }

    boost::system::error_code ec;
    boost::filesystem::copy_file
        (kServiceUserAgentPListSourcePath, kServiceUserAgentPListTargetPath, ec);
    if (ec) {
        log() << fmt::format ("[{}] failed to install service plist file\n", timestamp());
        return false;
    }

    return true;
}

int
main (int, const char*[])
{
    boost::filesystem::create_directory (kSharedConfigPath);

    try {
        std::ifstream appVersionFile;
        appVersionFile.open (kAppVersionFilePath, std::ios::in);

        /* Test to see if Synergy is installed. If not, remove thyself. */
        if (!appVersionFile.is_open()) {
            /* Remove all the service files */
            boost::system::error_code ec;
            boost::filesystem::remove (kServiceUserAgentPListTargetPath, ec);
            log() << fmt::format ("[{}] uninstalling user agent plist file... {}\n", 
                                  timestamp(), ec ? "failed" : "done");
            
            boost::filesystem::remove (kHelperPListPath, ec);
            log() << fmt::format ("[{}] uninstalling helper plist file... {}\n", 
                                  timestamp(), ec ? "failed" : "done");
            
            boost::filesystem::remove (kHelperExecPath, ec);
            log() << fmt::format ("[{}] uninstalling helper executable file... {}\n", 
                                  timestamp(), ec ? "failed" : "done");
            
            return EXIT_SUCCESS;
        }

        std::string version;
        std::getline (appVersionFile, version);
        appVersionFile.close();

        auto ts = timestamp();
        log() << fmt::format ("[{}] helper uid = {}, euid = {}, pid = {}\n", ts,
                              getuid(), geteuid(), getpid());
        log() << fmt::format ("[{}] installed helper revision = {}\n", ts, SYNERGY_REVISION);
        log() << fmt::format ("[{}] installed app revision = {}\n", ts, version);

        if (!installSynergyService()) {
            log() << fmt::format ("[{}] failed to install the service\n", timestamp());
        }
        else {
            log() << fmt::format ("[{}] installed the service successfully\n", timestamp());
        }

        log().close();
        return EXIT_SUCCESS;
    }
    catch (std::exception& ex) {
        log() << fmt::format ("[{}] Exception thrown: {}\n", timestamp(), ex.what());
    }
    catch (...) {
        log() << fmt::format ("[{}] Unknown exception thrown\n", timestamp());
    }

    log().close();
    return EXIT_FAILURE; 
}
