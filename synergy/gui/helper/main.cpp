#include <cstdlib>
#include <unistd.h>
#include <fstream>
#include <time.h>
#include <fmt/format.h>
#include <boost/filesystem.hpp>
#include <string>
#include <boost/process.hpp>
#include <stdexcept>
#include <cassert>
#include <time.h>

std::string const kSharedConfigPath ("/Users/Shared/Synergy");
std::string const kAppPath ("/Applications/Synergy.app");
std::string const kAppResourcePath (kAppPath + "/Contents/Resources");

auto const kAppClientExecPath = kAppPath + "/Contents/MacOS/synergyc";
auto const kAppVersionFilePath = kAppResourcePath + "/Version.txt";

auto const kHelperPListPath = "/Library/LaunchDaemons/com.symless.synergy.v2.ServiceHelper.plist";
auto const kHelperExecPath = "/Library/PrivilegedHelperTools/com.symless.synergy.v2.ServiceHelper";

std::string const kPreLoginAgentFilename = "com.symless.synergy.v2.PreLoginAgent.plist";
auto const kPreLoginAgentPListPath = "/Library/LaunchAgents/" + kPreLoginAgentFilename;
auto const kPreLoginAgentPListSourcePath = kAppResourcePath + "/" + kPreLoginAgentFilename;


static std::ofstream&
log() {
    static std::ofstream log_ (kSharedConfigPath + "/helper.log", std::ios::out | std::ios::app);
    return log_;
}


static bool
installPreLoginAgent() {
    if (boost::filesystem::exists (kPreLoginAgentPListPath)) {
        return true;
    }
    if (!boost::filesystem::exists (kPreLoginAgentPListSourcePath)) {
        return false;
    }

    boost::system::error_code ec;
    boost::filesystem::copy_file
        (kPreLoginAgentPListSourcePath, kPreLoginAgentPListPath, ec);
    if (ec) {
        return false;
    }

    /* Load the PreLogin agent */
    boost::process::ipstream launchd_out;
    boost::process::child launchd (fmt::format ("launchctl load {}", kPreLoginAgentPListPath),
                                   boost::process::std_out > launchd_out);
    std::string line;
    while (launchd_out && std::getline(launchd_out, line)) {
        if (!line.empty()) {
            log() << line << std::endl;
            line.clear();
        }
    }
    launchd.wait();
    return (EXIT_SUCCESS == launchd.exit_code());
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
            boost::filesystem::remove (kHelperPListPath, ec);
            boost::filesystem::remove (kHelperExecPath, ec);
            boost::filesystem::remove (kPreLoginAgentPListPath, ec);
            return EXIT_SUCCESS;
        }

        std::string version;
        std::getline (appVersionFile, version);
        appVersionFile.close();

        time_t t;
        time (&t);
        std::array<char, 32> ct_buf;
        auto ct = ctime_r (&t, ct_buf.data());
        assert (ct == ct_buf.data());
        std::replace (begin(ct_buf), end(ct_buf), '\n', '\0');

        log() << fmt::format ("[{}] installed helper revision = {}\n", ct, SYNERGY_REVISION);
        log() << fmt::format ("[{}] installed app revision = {}\n", ct, version);
        log() << fmt::format ("[{}] helper uid = {}, euid = {}, pid = {}\n", ct,
                              getuid(), geteuid(), getpid());

        if (!installPreLoginAgent()) {
            log() << fmt::format ("[{}] failed to install PreLoginAgent\n", ct);
        }

        /*boost::process::ipstream synergyc_out;
        boost::process::child synergyc (kAppClientExecPath, "-f", "192.168.1.93",
                                       boost::process::std_out > synergyc_out);
        std::string line;
        while (synergyc_out && std::getline(synergyc_out, line)) {
            // This should block and never spin
            if (!line.empty()) {
                log << line << std::endl;
                line.clear();
            }
        }

        synergyc.wait();*/
        log().close();
        return EXIT_SUCCESS;
    }
    catch (std::exception& ex) {
        log() << "Exception thrown: " << ex.what() << "\n";
    }
    catch (...) {
        log() << "Unknown exception thrown\n";
    }

    log().close();
    return EXIT_FAILURE; 
}
