#ifndef SERVICECONTROLLER_H
#define SERVICECONTROLLER_H

#include <memory>
#include <string>

class ServiceControllerImp;

class ServiceController
{
public:
    ServiceController();
    ~ServiceController();

    void parseArg(int argc, char* argv[]) {
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

    virtual void run() {
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

    void doRun();
    void install();
    void uninstall();

protected:
    std::unique_ptr<ServiceControllerImp> m_imp;
    bool m_install;
    bool m_uninstall;
};

#endif // SERVICECONTROLLER_H
