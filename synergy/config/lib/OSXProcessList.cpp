#include <assert.h>
#include <errno.h>
#include <stdbool.h>
#include <stdlib.h>
#include <stdio.h>
#include <sys/sysctl.h>
#include <iostream>
#include <boost/filesystem/path.hpp>
#include <libproc.h>
#include <signal.h>
#include <fmt/format.h>

// https://developer.apple.com/legacy/library/qa/qa2001/qa1123.html

// Returns a list of all BSD processes on the system.  This routine allocates
// the list and puts it in *procList and a count of the number of entries in *procCount.
// You are responsible for freeing this list (using "free" from System framework).
// On success, the function returns 0.
// On error, the function returns a BSD errno value.
static int
GetBSDProcessList(kinfo_proc** procList, size_t* procCount)
{
    assert (procList != NULL);
    assert (*procList == NULL);
    assert (procCount != NULL);

    *procCount = 0;

    // Declaring name as const requires us to cast it when passing it to
    // sysctl because the prototype doesn't include the const modifier.
    static const int name[] = { CTL_KERN, KERN_PROC, KERN_PROC_ALL, 0 };

    // We start by calling sysctl with result == NULL and length == 0.
    // That will succeed, and set length to the appropriate length.
    // We then allocate a buffer of that size and call sysctl again
    // with that buffer.  If that succeeds, we're done.  If that fails
    // with ENOMEM, we have to throw away our buffer and loop.  Note
    // that the loop causes use to call sysctl with NULL again; this
    // is necessary because the ENOMEM failure case sets length to
    // the amount of data returned, not the amount of data that
    // could have been returned.

    kinfo_proc* result = nullptr;
    size_t length;
    int error;
    bool done = false;
    do {
        assert (result == NULL);

        // Call sysctl with a NULL buffer.
        length = 0;
        error = sysctl ((int*) name, (sizeof(name) / sizeof(*name)) - 1,
                        NULL, &length, NULL, 0);
        if (error == -1) {
            error = errno;
        }

        // Allocate an appropriately sized buffer based on the results
        // from the previous call.
        if (error == 0) {
            result = static_cast<decltype(result)> (malloc (length));
            if (result == NULL) {
                error = ENOMEM;
            }
        }

        // Call sysctl again with the new buffer.  If we get an ENOMEM
        // error, toss away our buffer and start again.
        if (error == 0) {
            error = sysctl ((int *) name, (sizeof(name) / sizeof(*name)) - 1,
                            result, &length, NULL, 0);
            if (error == -1) {
                error = errno;
            }
            if (error == 0) {
                done = true;
            } else if (error == ENOMEM) {
                assert (result != NULL);
                free (result);
                result = NULL;
                error = 0;
            }
        }
    } while (error == 0 && !done);

    // Clean up and establish post conditions.
    if (error != 0 && result != NULL) {
        free (result);
        result = NULL;
    }

    *procList = result;
    if (error == 0) {
        *procCount = length / sizeof(kinfo_proc);
    }

    assert ((error == 0) == (*procList != NULL));
    return error;
}

static boost::filesystem::path
getPidPath (pid_t const pid = getpid()) {
    char buf[PROC_PIDPATHINFO_MAXSIZE];
    auto const ret = proc_pidpath (pid, buf, sizeof(buf));
    if (ret <= 0) {
        throw std::runtime_error (fmt::format ("Couldn't get executable file path for PID {}", pid));
    } else {
        return boost::filesystem::path (buf);
    }
}

static bool
isInstalledSynergyComponent (boost::filesystem::path const& path) {
    static boost::filesystem::path const kInstallPath = "/Applications/Synergy.app";
    using std::begin;
    using std::end;
    return (std::mismatch (begin(kInstallPath), end(kInstallPath),
                           begin(path), end(path)).first == end(kInstallPath));
}

bool
iAmInstalled() {
    return isInstalledSynergyComponent (getPidPath());
}

void
killInstalledSynergyComponents() {
    kinfo_proc* procs = nullptr;
    size_t count = 0;
    auto const err = GetBSDProcessList (&procs, &count);

    for (auto i = 0; i < count; ++i) {
        auto const pid = procs[i].kp_proc.p_pid;
        boost::filesystem::path path;
        try {
            path = getPidPath(pid);
        } catch (...) {
            continue;
        }
        if (isInstalledSynergyComponent (path)) {
            std::cout << fmt::format ("Killing PID {} = {} ... ", pid, path.string());
            std::cout << ((0 == kill (pid, SIGKILL)) ? "OK" : "Failed");
            std::cout << "\n";
        }
    }
    free (procs);
}
