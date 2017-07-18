#include "IServiceController.h"

#include <string>

static char* kServiceProcessName = "synergy-service";

IServiceController::IServiceController() :
    m_install(false),
    m_uninstall(false)
{

}

IServiceController::~IServiceController()
{

}

void IServiceController::parseArg(int argc, char *argv[])
{
    for (int i = 1; i < argc; ++i) {
        std::string arg(argv[i]);
        if (arg == "/install" || arg == "--install") {
            m_install = true;
        }
        else if (arg == "/uninstall" || arg == "--uninstall") {
            m_uninstall = true;
        }
    }
}

char *IServiceController::processName() const
{
    return kServiceProcessName;
}

void IServiceController::run()
{
    if (m_install) {
        install();
    }
    else if (m_uninstall) {
        uninstall();
    }
    else {
        doRun();
    }
}
