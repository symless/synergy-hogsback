#include <zmq.hpp>


int main() {
    zmq::context_t ctx;
    zmq::socket_t mcast_pub (ctx, ZMQ_PUB);
}
