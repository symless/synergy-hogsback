#include "../ServiceController.h"

//
//  ServiceControllerImp for Mac
//

class ServiceControllerImp {

};

//
//  ServiceController for Mac
//

ServiceController::ServiceController() :
    m_install(false),
    m_uninstall(false),
    m_foreground(false),
#if defined(SIGQUIT)
    m_terminationSignals(m_threadIoService, SIGTERM, SIGINT, SIGQUIT)
#else
    m_terminationSignals(m_threadIoService, SIGTERM, SIGINT)
#endif
{
    
}

ServiceController::~ServiceController()
{

}

void ServiceController::doRun()
{
    setupTerminationSignals();
    m_worker->start();
}

void ServiceController::install()
{
    // there is no implementatioon for manual installing on Mac
}

void ServiceController::uninstall()
{
    // there is no implementatioon for manual uninstalling on Mac
}

void ServiceController::shutdown()
{

}
