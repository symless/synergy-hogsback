#include <zmq.hpp>
#include <iostream>

int main() {
   zmq::context_t ctx;
   zmq::socket_t socket (ctx, ZMQ_SUB);
   socket.connect ("norm://en0;224.1.1.1:5555");
   socket.setsockopt (ZMQ_SUBSCRIBE, "", 0);

   zmq::message_t msg;
   zmq::pollitem_t pollit {static_cast<void*>(socket), -1, ZMQ_POLLIN, 0};
   do {
       auto n = zmq::poll (&pollit, 1, -1);
       if (n <= 0) {
           break;
       }
       socket.recv (&msg, ZMQ_RCVMORE);
//       std::cout << std::string(static_cast<char const*>(msg.data()), 
//                    msg.size()) << std::endl;
       socket.recv (&msg);
       std::cout << std::string(static_cast<char const*>(msg.data()), 
                    msg.size()) << std::endl;

   } while (true);
}

