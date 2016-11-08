#include "service.hpp"
#include <boost/optional.hpp>
#include <fmt/format.h>
#include <iostream>
#include <thread>
#include <zmq.hpp>

static auto const INPROC_ADDR_FMT    = "inproc://Synergy_{}";
static auto const LOCAL_TCP_ADDR_FMT = "tcp://127.0.0.1:{}";
static auto const MULTICAST_ADDR_FMT = "epgm://{};230.83.89.78:{}";

using boost::optional;

struct ServiceImpl {
    zmq::context_t zmq_ctx;
    optional<zmq::socket_t> control;
    optional<zmq::socket_t> gui;
    optional<zmq::socket_t> core;
    optional<zmq::socket_t> service_log;
    std::thread proxy_thread;
    uint16_t base_port;
    bool started = false;
    void proxy_loop ();
};


Service::Service (uint16_t const base_port)
    : impl_ (std::make_shared<ServiceImpl> ()) {
    if ((base_port < 1024) || (base_port > 65495)) {
        throw;
    }

    /* Control interface */
    impl_->control.emplace (impl_->zmq_ctx, ZMQ_REP);
    impl_->control->setsockopt (ZMQ_LINGER, 10000);
    impl_->control->bind (fmt::format (LOCAL_TCP_ADDR_FMT, base_port + 10));

    /* GUI interface */
    impl_->gui.emplace (impl_->zmq_ctx, ZMQ_XPUB);
    impl_->gui->setsockopt (ZMQ_LINGER, 10000);
    impl_->gui->bind (fmt::format (LOCAL_TCP_ADDR_FMT, base_port + 20));

    auto inproc_addr = fmt::format (INPROC_ADDR_FMT, base_port);

    /* Core interface */
    impl_->core.emplace (impl_->zmq_ctx, ZMQ_XSUB);
    impl_->core->setsockopt (ZMQ_LINGER, 10000);
    impl_->core->bind (inproc_addr);
    impl_->core->bind (fmt::format (LOCAL_TCP_ADDR_FMT, base_port + 30));

    /* Thread safe logging channel */
    impl_->service_log.emplace (impl_->zmq_ctx, ZMQ_PUB);
    impl_->service_log->setsockopt (ZMQ_LINGER, 10000);
    impl_->service_log->connect (inproc_addr);

    impl_->proxy_thread = std::thread ([&]() { impl_->proxy_loop (); });
    impl_->base_port    = base_port;
}

Service::~Service () {
    if (impl_) {
        impl_->service_log->close ();
        impl_->control->close ();
        impl_->zmq_ctx.close ();
        impl_->proxy_thread.join ();
    }
}

void
ServiceImpl::proxy_loop () {
    if (!gui || !core) {
        throw;
    }
    try {
        zmq::proxy (
            static_cast<void*> (*core), static_cast<void*> (*gui), nullptr);
    } catch (zmq::error_t& error) {
        if (error.num () != ETERM) {
            throw error;
        }
    }
    core->close ();
    gui->close ();
}

void
Service::start () {
    if (impl_->started) {
        return;
    }

    impl_->started = true;

    /* Multicast sockets */
    zmq::socket_t mcast_sub (impl_->zmq_ctx, ZMQ_SUB);
    zmq::socket_t mcast_pub (impl_->zmq_ctx, ZMQ_PUB);
    auto const mcast_addr =
        fmt::format (MULTICAST_ADDR_FMT, "192.168.1.2", impl_->base_port + 40);
    mcast_sub.connect (mcast_addr);
    mcast_pub.connect (mcast_addr);

    std::vector<zmq_pollitem_t> poll_set = {
        {static_cast<void*> (*impl_->control), -1, ZMQ_POLLIN, 0},
        {static_cast<void*> (mcast_sub), -1, ZMQ_POLLIN, 0}};

    do {
        auto const n = zmq::poll (poll_set.data (),
                                  poll_set.size (),
                                  std::chrono::milliseconds (10000));
        if (n) {
            if (poll_set[0].revents & ZMQ_POLLIN) {
            }
            if (poll_set[1].revents & ZMQ_POLLIN) {
            }
        }
    } while (true);
}

int
main (int, char* []) {
    Service service;
    service.start ();
    return 0;
}
