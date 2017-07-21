#include "../ServiceController.h"

//
// ServiceController for Linux
//

class ServiceControllerImp {

};

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
    m_worker = std::make_shared<ServiceWorker>(m_threadIoService);
}
ServiceController::~ServiceController()
{

}

void ServiceController::doRun()
{

}

void ServiceController::install()
{

}

void ServiceController::uninstall()
{

}
