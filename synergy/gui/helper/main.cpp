#include <syslog.h>
#include <unistd.h>
#include <cstdio>
#include <cstdlib>
#include <fstream>
#include <time.h>

int
main (int, const char*[])
{
    std::ofstream ofs ("/Users/Andrew/test.txt");
    auto t = time(NULL);
    ofs << "test " << ctime(&t) << "\n";
    ofs.close();
    syslog (LOG_NOTICE, "Hello world! uid = %d, euid = %d, pid = %d\n", (int) getuid(), (int) geteuid(), (int) getpid());
    sleep (10);
    return EXIT_SUCCESS;
}

