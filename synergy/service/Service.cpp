#include "Service.h"

#ifdef _WIN32
#include "MSWindowsServiceController.h"
#elif __APPLE__
#include "OSXServiceController.h"
#else
#include "XWindowsServiceController.h"
#endif

Service::Service()
{
    m_controller = new MSWindowsServiceController();
}

Service::~Service()
{
    delete m_controller;
}

void
Service::parseArg(int argc, char *argv[])
{
    m_controller->parseArg(argc, argv);
}

void Service::run()
{
    m_controller->run();
}
