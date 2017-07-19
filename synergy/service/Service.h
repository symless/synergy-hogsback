#ifndef SERVICE_H
#define SERVICE_H

class ServiceController;
class ServiceController;

class Service
{
public:
    Service();
    ~Service();

    void parseArg(int argc, char* argv[]);
    void run();

private:
    ServiceController* m_controller;
};

#endif // SERVICE_H
