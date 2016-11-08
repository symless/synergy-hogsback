#ifndef SERVICE_HPP
#define SERVICE_HPP

#include <cstdint>
#include <memory>

struct ServiceImpl;

class Service {
public:
    explicit Service (uint16_t base_port = 24800);
    ~Service ();
    void start ();

private:
    std::shared_ptr<ServiceImpl> impl_;
    void update_multicast_set();
    void say_hello();
};

#endif // SERVICE_HPP
