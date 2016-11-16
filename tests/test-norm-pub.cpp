#include <zmq.hpp>
#include <iostream>
#include <unistd.h>

int main() {
   zmq::context_t ctx;
   zmq::socket_t socket (ctx, ZMQ_PUB);
   socket.connect ("norm://en0;224.1.1.1:5555");

   std::string hw ("Hello world");
   do {
       zmq::message_t hdr;
       zmq::message_t msg (hw.data(), hw.size());
       socket.send (hdr, ZMQ_SNDMORE);
       socket.send (msg);
       ::sleep(3);
   } while (true);
}
