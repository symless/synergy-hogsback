#pragma once

#include <boost/asio.hpp>
#include <boost/signals2.hpp>
#include <memory>
#include <string>
#include <vector>

class SessionMonitor final {
public:
    explicit SessionMonitor (boost::asio::io_service&);
    ~SessionMonitor ();

    void start ();
    void stop ();

    // TODO: make these functions private/impl
    void poll ();
    std::vector<std::string> sessions ();

private:
    struct Impl;
    std::unique_ptr<Impl> impl_;
    boost::asio::io_service& ioService_;

public:
    boost::signals2::signal<void(std::string)> activeUserChanged;
    boost::signals2::signal<void(std::string)> activeDisplayChanged;
};

