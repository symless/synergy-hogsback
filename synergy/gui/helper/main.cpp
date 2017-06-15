#include <cstdlib>
#include <unistd.h>
#include <fstream>
#include <time.h>
#include <fmt/format.h>
#include <boost/filesystem.hpp>
#include <string>

int
main (int, const char*[])
{    
    std::ofstream testlog ("/Users/Andrew/test.txt");
    std::ifstream versionFile;
    std::string version ("none");
    versionFile.open ("/Applications/Synergy.app/Contents/Resources/Version.txt");
    if (versionFile.is_open()) {
        std::getline (versionFile, version);
    }

    auto t = time(NULL);
    testlog << fmt::format ("installed revision = {}, time = {} uid = {}, "
                            "euid = {}, pid = {}\n", ctime(&t), getuid(), geteuid(), getpid());
    testlog.flush();
    return EXIT_SUCCESS;
}

