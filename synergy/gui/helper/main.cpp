#include <syslog.h>
#include <unistd.h>
#include <cstdio>
#include <cstdlib>

int
main (int, const char*[])
{
    syslog (LOG_NOTICE, "Hello world! uid = %d, euid = %d, pid = %d\n", (int) getuid(), (int) geteuid(), (int) getpid());
    sleep (10);
    return EXIT_SUCCESS;
}

