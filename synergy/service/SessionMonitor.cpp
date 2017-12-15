#include <boost/asio.hpp>
#include <boost/optional.hpp>
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
    std::vector<std::string> sessions ();
    void poll ();

private:
    struct Impl;
    std::unique_ptr<Impl> impl_;
    boost::asio::io_service& ios_;

public:
    boost::signals2::signal<void(boost::optional<std::string>)>
        activeUserChanged;
    
    boost::signals2::signal<void(boost::optional<std::string>)>
        activeDisplayChanged;
};

#include <algorithm>
#include <boost/asio/posix/stream_descriptor.hpp>
#include <boost/asio/spawn.hpp>
#include <boost/scope_exit.hpp>
#include <cassert>
#include <cstdlib>
#include <cstring>
#include <iostream>
#include <stdexcept>
#include <systemd/sd-login.h>

struct SessionMonitor::Impl {
    sd_login_monitor* monitor = nullptr;
    boost::asio::posix::stream_descriptor fd;
    boost::asio::null_buffers nullBuffers;
    boost::optional<uid_t> activeUser;
    boost::optional<std::string> activeDisplay;
    bool stopped = true;

    Impl (boost::asio::io_service& ios) : fd (ios) {
        if (sd_login_monitor_new ("session", &monitor) < 0) {
            throw std::runtime_error ("Failed to create sd-login monitor");
        }
        fd.assign (sd_login_monitor_get_fd (monitor));
    }

    ~Impl () noexcept {
        fd.release ();
        sd_login_monitor_unref (monitor);
    }
};

template <typename T>
static inline void
free_and_null (T*& ptr) noexcept {
    if (ptr) {
        ::free (ptr);
        ptr = nullptr;
    }
}

template <typename T, typename Size>
static inline void
free_and_null_array (T**& arr_in, Size& size_in) noexcept {
    auto const arr = arr_in;
    if (arr) {
        auto const size = size_in;
        for (auto i = 0; i < size; ++i) {
            free_and_null (arr[i]);
        }
        free_and_null (arr_in);
    }
    size_in = 0;
}

SessionMonitor::SessionMonitor (boost::asio::io_service& ios)
    : impl_ (std::make_unique<Impl> (ios)), ios_ (ios) {
    start ();
}

SessionMonitor::~SessionMonitor () = default;

void
SessionMonitor::start () {
    if (!this->impl_->stopped) {
        return;
    }

    boost::asio::spawn (ios_, [this](auto ctx) {
        while (!this->impl_->stopped) {
            boost::system::error_code ec;
            this->impl_->fd.async_read_some (impl_->nullBuffers, ctx[ec]);
            if (ec == boost::asio::error::operation_aborted) {
                this->impl_->stopped = true;
                break;
            } else if (ec) {
                break;
            }
            sd_login_monitor_flush (this->impl_->monitor);
            this->poll ();
        }
    });

    this->impl_->stopped = false;
}

void
SessionMonitor::stop () {
    if (impl_->stopped) {
        return;
    }
    impl_->fd.cancel ();
    ios_.poll ();
    assert (impl_->stopped);
}

std::vector<std::string>
SessionMonitor::sessions () {
    char** sd_sessions   = nullptr;
    int sd_session_count = 0;

    BOOST_SCOPE_EXIT (&sd_sessions, &sd_session_count) {
        free_and_null_array (sd_sessions, sd_session_count);
    }
    BOOST_SCOPE_EXIT_END

    sd_session_count = std::max (sd_get_sessions (&sd_sessions), 0);
    return std::vector<std::string> (sd_sessions,
                                     sd_sessions + sd_session_count);
}

void
SessionMonitor::poll () {
    for (auto& session : sessions ()) {
        if (sd_session_is_active (session.c_str ())) {
            char* type = nullptr;
            BOOST_SCOPE_EXIT (&type) {
                free_and_null (type);
            }
            BOOST_SCOPE_EXIT_END

            if (sd_session_get_type (session.c_str (), &type) < 0) {
                throw std::runtime_error (
                    "Couldn't determine type of active session");
            }
            if (strcmp (type, "x11") != 0) {
                return;
            }

            uid_t user;
            if (sd_session_get_uid (session.c_str (), &user) < 0) {
                throw std::runtime_error (
                    "Couldn't determine user of active session");
            }
            if (!impl_->activeUser || (*impl_->activeUser != user)) {
                impl_->activeUser = user;
                activeUserChanged (std::to_string (user));
            }

            char* display = nullptr;
            BOOST_SCOPE_EXIT (&display) {
                free_and_null (display);
            }
            BOOST_SCOPE_EXIT_END
            if (sd_session_get_display (session.c_str (), &display) < 0) {
                throw std::runtime_error ("Couldn't determine active display");
            }
            if (!impl_->activeDisplay || (*impl_->activeDisplay != display)) {
                impl_->activeDisplay = display;
                activeDisplayChanged (impl_->activeDisplay);
            }
            return;
        }
    }
}

#include <iostream>
#include <boost/optional/optional_io.hpp>

int
main () {
    boost::asio::io_service ios;
    SessionMonitor sm (ios);

    sm.activeUserChanged.connect([](auto user) {
    	std::cout << "new active user: " << user << "\n";
    });

    sm.activeDisplayChanged.connect([](auto display) {
    	std::cout << "new active display: " << display << "\n";
    });

    ios.run ();
}
