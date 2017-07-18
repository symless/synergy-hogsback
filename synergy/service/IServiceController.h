#ifndef ISERVICECONTROLLER_H
#define ISERVICECONTROLLER_H


class IServiceController
{
public:
    IServiceController();
    virtual ~IServiceController();

    void parseArg(int argc, char* argv[]);
    char* processName() const;

    virtual void run();
    virtual void doRun() = 0;
    virtual void install() = 0;
    virtual void uninstall() = 0;

protected:
    bool m_install;
    bool m_uninstall;
};

#endif // ISERVICECONTROLLER_H
