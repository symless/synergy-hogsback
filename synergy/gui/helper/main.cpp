#include <cstdlib>
#include <unistd.h>
#include <fstream>
#include <time.h>
#include <fmt/format.h>
#include <boost/filesystem.hpp>
#include <string>
#include <boost/process.hpp>
#include <stdexcept>

std::string const kSharedConfigPath ("/Users/Shared/Synergy");
std::string const kAppPath ("/Applications/Synergy.app");

auto const kAppVersionFilePath = kAppPath + "/Contents/Resources/Version.txt";
auto const kHelperPListPath = "/Library/LaunchDaemons/com.symless.synergy.v2.ServiceHelper.plist";
auto const kHelperExecPath = "/Library/PrivilegedHelperTools/com.symless.synergy.v2.ServiceHelper";

int
main (int, const char*[])
{
    boost::filesystem::create_directory (kSharedConfigPath);
    std::ofstream log (kSharedConfigPath + "/helper.log", std::ios::out | std::ios::app);
    try {
        std::ifstream appVersionFile;
        appVersionFile.open (kAppVersionFilePath, std::ios::in);

        /* Test to see if Synergy is installed. If not, remove thyself. */
        if (!appVersionFile.is_open()) {
            /* First call launchctl to unload the helper */
            boost::process::ipstream launchd_out;
            boost::process::child launchd (fmt::format("launchctl unload {}", kHelperPListPath),
                                           boost::process::std_out > launchd_out);
            std::string line;
            while (launchd_out && std::getline(launchd_out, line)) {
                log.write (line.data(), line.size());
            }
            launchd.wait();

            /* ... then remove the files */
            boost::system::error_code ec;
            boost::filesystem::remove (kHelperPListPath, ec);
            boost::filesystem::remove (kHelperExecPath, ec);

            return EXIT_SUCCESS;
        }

        std::string version;
        std::getline (appVersionFile, version);
        appVersionFile.close();

        auto t = time(NULL);
        auto ct = ctime(&t);
        log << fmt::format ("[{}] installed helper revision = {}\n", ct, SYNERGY_REVISION);
        log << fmt::format ("[{}] installed app revision = {}\n", ct, version);
        log << fmt::format ("[{}] helper uid = {}, euid = {}, pid = {}\n", ct,
                            getuid(), geteuid(), getpid());
        log.close();
        return EXIT_SUCCESS;
    } catch (std::exception& ex) {
        log << "Exception thrown: " << ex.what() << "\n";
    } catch (...) {
        log << "Unknown exception thrown\n";
    }
    log.close();
    return EXIT_FAILURE; 
}
