#ifndef SERVICE_H
#define SERVICE_H

class IServiceController;
class IServiceController;

class Service
{
public:
    Service();
    ~Service();

    void parseArg(int argc, char* argv[]);
    void run();

private:
    IServiceController* m_controller;
};

#endif // SERVICE_H
