#include <synergy/common/DirectoryManager.h>
#include <synergy/common/CrashHandler.h>
#include <boost/predef.h>
#include <client/crashpad_client.h>
#include <client/crash_report_database.h>
#include <client/settings.h>
#include <iostream>

static auto const CRASHPAD_TOKEN =
    "cc4db6ef2d4731b276b0a111979336443dc0372624effb79d8b29b26a200e1c6";

bool
startCrashHandler()
{
    auto& directoryManager = *DirectoryManager::instance();

#if (BOOST_OS_WINDOWS)
    auto dbPath = directoryManager.crashDumpDir().wstring();
    auto handlerPath = (directoryManager.installedDir()
                            / "crashpad_handler.exe").wstring();
#elif (BOOST_OS_MACOS)
    auto dbPath = directoryManager.crashDumpDir().string();
    auto handlerPath = (directoryManager.installedDir()
                            / "crashpad_handler").string();
#endif

    using namespace crashpad;
    base::FilePath db (dbPath);
    base::FilePath handler (handlerPath);

    /* Enable uploads */
    {
        auto crashReportDatabase = CrashReportDatabase::Initialize(db);
        crashReportDatabase->GetSettings()->SetUploadsEnabled(true);
    }

    std::string url("https://synergy.sp.backtrace.io:6098");
    std::map<std::string, std::string> annotations;
    annotations["token"] = CRASHPAD_TOKEN;
    annotations["format"] = "minidump";

    std::vector<std::string> arguments;
    arguments.push_back ("--no-rate-limit");

    CrashpadClient client;
    bool rc = client.StartHandler (handler, db, db, url, annotations, arguments,
        true, /* restartable */
        false /* asynchronous_start */
    );
    if (!rc) {
        return false;
    }

    return true;
}
