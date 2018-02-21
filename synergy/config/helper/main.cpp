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
#include <pwd.h>
#include <sys/stat.h>

std::string const kSharedConfigPath ("/Users/Shared/Synergy");
std::string const kSharedUserData (kSharedConfigPath + "/user.cfg");
std::string const kAppPath ("/Applications/Synergy.app");
std::string const kAppResourcePath (kAppPath + "/Contents/Resources");
auto const kAppVersionFilePath = kAppResourcePath + "/Version.txt";

std::string const kHelperName = "com.symless.synergy.v2.ServiceHelper";
auto const kHelperPListPath = "/Library/LaunchDaemons/" + kHelperName + ".plist";
auto const kHelperExecPath = "/Library/PrivilegedHelperTools/" + kHelperName;

std::string const kServiceUserAgentFilename = "com.symless.synergy.synergy-service.plist";
auto const kServiceUserAgentPListTargetPath = "/Library/LaunchAgents/" + kServiceUserAgentFilename;
auto const kServiceUserAgentPListSourcePath = kAppResourcePath + "/" + kServiceUserAgentFilename;

// This file was made redundant in v2.0-beta4. If you're reading this in 2018 or later, you can
// probably remove this.
auto const kServiceLegacyUserAgentPListPath = "/Library/LaunchAgents/com.symless.synergy.v2.synergyd.plist";

static std::ofstream&
log() {
    static std::ofstream lofs(kSharedConfigPath + "/helper.log", std::ios::out | std::ios::app);
    assert (lofs.is_open());
    return lofs;
}

static std::ofstream&
conf_out() {
    static std::ofstream conf_out(kSharedUserData, std::ios::out | std::ios::trunc);
    assert (conf_out.is_open());
    return conf_out;
}

static std::ifstream&
conf_in() {
    static std::ifstream conf_in(kSharedUserData, std::ios::in);
    assert (conf_in.is_open());
    return conf_in;
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
installSynergyService(bool const force = false)
{
    if (!force && boost::filesystem::exists (kServiceUserAgentPListTargetPath)) {
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
main (int argc, const char* argv[], const char* env[]) {
    boost::filesystem::create_directory(kSharedConfigPath);

    while (*env != NULL) {
        log() << fmt::format ("[{}] var - {}\n", timestamp(), *env);
        env++;
    }

    try {
        std::ifstream appVersionFile;
        appVersionFile.open (kAppVersionFilePath, std::ios::in);

        boost::system::error_code ec;
        boost::filesystem::remove (kServiceLegacyUserAgentPListPath, ec);

        /* Test to see if Synergy is installed. If not, remove thyself. */
        if (!appVersionFile.is_open()) {
            /* Remove all the service files */
            boost::filesystem::remove (kServiceUserAgentPListTargetPath, ec);
            log() << fmt::format ("[{}] uninstalling user agent plist file... {}\n", 
                                  timestamp(), ec ? "Failed" : "OK");
            
            boost::filesystem::remove (kHelperPListPath, ec);
            log() << fmt::format ("[{}] uninstalling helper plist file... {}\n",
                                  timestamp(), ec ? "Failed" : "OK");
            
            boost::filesystem::remove (kHelperExecPath, ec);
            log() << fmt::format ("[{}] uninstalling helper executable... {}\n",
                                  timestamp(), ec ? "Failed" : "OK");

            // remove the preference and user files too
            std::string userHome;
            conf_in() >> userHome;
            conf_in().close();

            log() << fmt::format ("[{}] userHome = {}\n",
                                  timestamp(),
                                  userHome);

            auto const kUserFilesDirectory = userHome + "/Library/Synergy";
            auto const kUserPreferencesDirectory = userHome + "/Library/Preferences/Symless";
            auto const kUserPreferencesConfigPlistFile = userHome + "/Library/Preferences/com.https-symless-com.Synergy.plist";

            boost::filesystem::remove_all (kUserFilesDirectory, ec);
            log() << fmt::format ("[{}] removing {} {}\n",
                                  timestamp(),
                                  kUserFilesDirectory,
                                  ec ? "Failed" : "OK");

            boost::filesystem::remove_all (kUserPreferencesDirectory, ec);
            log() << fmt::format ("[{}] removing {} {}\n",
                                  timestamp(),
                                  kUserPreferencesDirectory,
                                  ec ? "Failed" : "OK");

            boost::filesystem::remove (kUserPreferencesConfigPlistFile, ec);
            log() << fmt::format ("[{}] removing {} {}\n",
                                  timestamp(),
                                  kUserPreferencesConfigPlistFile,
                                  ec ? "Failed" : "OK");

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

        // grab and store the home directory of the user who installed Synergy
        struct stat fileInfo;
        stat(kAppPath.data(), &fileInfo);
        log() << fmt::format ("[{}] installing user homedir is {} \n", ts, getpwuid(fileInfo.st_uid)->pw_dir);
        conf_out() << getpwuid(fileInfo.st_uid)->pw_dir;
        conf_out().flush();
        conf_out().close();

        

        if (!installSynergyService (version != SYNERGY_REVISION)) {
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

    conf_out().close();
    conf_in().close();
    log().close();
    return EXIT_FAILURE; 
}
