#include "service.hpp"
#include "screen_info.hpp"
#include <boost/optional.hpp>
#include <chrono>
#include <fmt/format.h>
#include <iostream>
#include <multicast.hpp>
#include <thread>
#include <tuple>
#include <zmq.hpp>

using boost::optional;

static auto const INPROC_ADDR_FMT    = "inproc://Synergy_{}";
static auto const LOCAL_TCP_ADDR_FMT = "tcp://127.0.0.1:{}";

struct ServiceImpl {
    zmq::context_t zmq_ctx;
    optional<zmq::socket_t> control;
    optional<zmq::socket_t> gui;
    optional<zmq::socket_t> core;
    optional<zmq::socket_t> service_log;
    optional<zmq::socket_t> mcast_pub;
    optional<zmq::socket_t> mcast_sub;
    std::vector<MulticastInterface> multicast_interfaces;
    std::thread core_gui_proxy_thread;
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

    auto const inproc_addr = fmt::format (INPROC_ADDR_FMT, base_port);

    /* Core interface */
    impl_->core.emplace (impl_->zmq_ctx, ZMQ_XSUB);
    impl_->core->setsockopt (ZMQ_LINGER, 10000);
    impl_->core->bind (inproc_addr);
    impl_->core->bind (fmt::format (LOCAL_TCP_ADDR_FMT, base_port + 30));

    /* Thread safe logging channel */
    impl_->service_log.emplace (impl_->zmq_ctx, ZMQ_PUB);
    impl_->service_log->setsockopt (ZMQ_LINGER, 10000);
    impl_->service_log->connect (inproc_addr);

    /* Multicast interface */
    impl_->mcast_pub.emplace (impl_->zmq_ctx, ZMQ_PUB);
    impl_->mcast_pub->setsockopt (ZMQ_MULTICAST_HOPS, 16);
    impl_->mcast_sub.emplace (impl_->zmq_ctx, ZMQ_SUB);
    impl_->mcast_sub->setsockopt (ZMQ_SUBSCRIBE, "", 0);

    impl_->core_gui_proxy_thread =
        std::thread ([&]() { impl_->proxy_loop (); });
    impl_->base_port = base_port;
}

Service::~Service () {
    if (impl_) {
        impl_->service_log->close ();
        impl_->control->close ();
        impl_->zmq_ctx.close ();
        impl_->core_gui_proxy_thread.join ();
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
Service::say_hello () {
    std::string hello_world ("Hello world");
    zmq::message_t hdr ("HELLO", 5);
    zmq::message_t msg (hello_world.data (), hello_world.size ());
    impl_->mcast_pub->send (hdr, ZMQ_SNDMORE);
    impl_->mcast_pub->send (msg);
}

void
Service::update_multicast_set () {
    auto multicast_interfaces = get_all_multicast_interfaces ();

    for (auto& ifc : multicast_interfaces) {
        std::cout << "Detected: " << ifc.name << " -> " << ifc.primary_ip
                  << std::endl;
    }

    std::vector<MulticastInterface> added;
    std::set_difference (begin (multicast_interfaces),
                         end (multicast_interfaces),
                         begin (impl_->multicast_interfaces),
                         end (impl_->multicast_interfaces),
                         std::back_inserter (added));

    std::vector<MulticastInterface> removed;
    std::set_difference (begin (impl_->multicast_interfaces),
                         end (impl_->multicast_interfaces),
                         begin (multicast_interfaces),
                         end (multicast_interfaces),
                         std::back_inserter (removed));

    for (auto& rm_ifc : removed) {
        std::cout << "Removed: " << rm_ifc.name << " -> " << rm_ifc.primary_ip;
        auto addr = fmt::format (
            MULTICAST_ADDR_FMT, rm_ifc.primary_ip, impl_->base_port + 40);
        std::cout << " -> " << addr << std::endl;
        impl_->mcast_sub->disconnect (addr);
        impl_->mcast_pub->disconnect (addr);
    }

    /* Reserve space for the interfaces to be added */
    impl_->multicast_interfaces.reserve (impl_->multicast_interfaces.size () +
                                         added.size ());
    auto mp = impl_->multicast_interfaces.end ();
    for (auto& new_ifc : added) {
        std::cout << "Added: " << new_ifc.name << " -> " << new_ifc.primary_ip;
        auto addr = fmt::format (
            MULTICAST_ADDR_FMT, new_ifc.primary_ip, impl_->base_port + 40);
        std::cout << " -> " << addr << std::endl;
        impl_->mcast_sub->connect (addr);
        impl_->mcast_pub->connect (addr);
        impl_->multicast_interfaces.push_back (std::move (new_ifc));
    }

    /* Merge in all the newly discovered interfaces */
    std::inplace_merge (begin (impl_->multicast_interfaces),
                        mp,
                        end (impl_->multicast_interfaces));

    /* Remove all the interfaces that no longer seem to be present */
    added.clear ();
    std::set_difference (begin (impl_->multicast_interfaces),
                         end (impl_->multicast_interfaces),
                         begin (removed),
                         end (removed),
                         std::back_inserter (added));
    impl_->multicast_interfaces = std::move (added);
}

void
Service::process_multicast_messages () {
    zmq::message_t msg;
    impl_->mcast_sub->recv (&msg);

    if ((msg.size () == 5) && (0 == std::memcmp (msg.data (), "HELLO", 5))) {
        auto more_to_recv = impl_->mcast_sub->getsockopt<int> (ZMQ_RCVMORE);
        assert (more_to_recv);
        impl_->mcast_sub->recv (&msg);
        std::cout << std::string (static_cast<char const*> (msg.data ()),
                                  msg.size ())
                  << std::endl;
    } else {
        while (impl_->mcast_sub->getsockopt<int> (ZMQ_RCVMORE)) {
            impl_->mcast_sub->recv (&msg);
        }
    }
}

void
Service::process_control_messages () {
}

void
Service::start () {
    if (impl_->started) {
        return;
    }

    impl_->started = true;

    std::vector<zmq_pollitem_t> poll_set = {
        {static_cast<void*> (*impl_->control), -1, ZMQ_POLLIN, 0},
        {static_cast<void*> (*impl_->mcast_sub), -1, ZMQ_POLLIN, 0}};

    do {
        auto const t1 = std::chrono::high_resolution_clock::now ();
        auto const n  = zmq::poll (poll_set.data (),
                                  poll_set.size (),
                                  std::chrono::milliseconds (5000));
        if (n > 0) {
            if (poll_set[0].revents & ZMQ_POLLIN) {
                process_control_messages ();
            }
            if (poll_set[1].revents & ZMQ_POLLIN) {
                process_multicast_messages ();
            }
        }

        auto const t2 = std::chrono::high_resolution_clock::now ();
        if ((t2 - t1) >= std::chrono::seconds (5)) {
            update_multicast_set ();
            say_hello ();
        }
    } while (true);
}

int
main (int, char* []) {
    Service service;
    service.start ();
    return 0;
}
