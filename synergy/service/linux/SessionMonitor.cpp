#include <synergy/service/SessionMonitor.h>

#include <algorithm>
#include <boost/asio/posix/stream_descriptor.hpp>
#include <boost/asio/spawn.hpp>
#include <boost/scope_exit.hpp>
#include <cassert>
#include <cstdlib>
#include <cstring>
#include <stdexcept>
#include <systemd/sd-login.h>
#include <synergy/service/ServiceLogs.h>
#include <fmt/ostream.h>
#include <boost/optional.hpp>
#include <boost/optional/optional_io.hpp>

struct SessionMonitor::Impl {
    sd_login_monitor* monitor = nullptr;
    boost::asio::posix::stream_descriptor fd;
    boost::asio::null_buffers nullBuffers;
    boost::optional<std::string> activeXSession;
    boost::optional<std::string> activeXDisplay;
    boost::optional<uid_t> activeUser;
    bool stopped = true;

    Impl (boost::asio::io_service& ios) : fd (ios) {
        if (sd_login_monitor_new ("session", &monitor) < 0) {
            serviceLog()->critical ("Failed to instantiate systemd session "
                                    "monitor");
            return;
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
    : impl_ (std::make_unique<Impl> (ios)), ioService_ (ios) {
}

SessionMonitor::~SessionMonitor () = default;

void
SessionMonitor::start () {
    if (!this->impl_->stopped || !this->impl_->monitor) {
        return;
    }

    boost::asio::spawn (ioService_, [this](auto ctx) {
        while (!this->impl_->stopped) {
            boost::system::error_code ec;
            this->impl_->fd.async_read_some (impl_->nullBuffers, ctx[ec]);
            if (ec == boost::asio::error::operation_aborted) {
                this->impl_->stopped = true;
                serviceLog()->info ("Session monitor stopped");
                break;
            } else if (ec) {
                serviceLog()->error ("Polling session monitor  failed with "
                                     "error: {}", ec.message());
                serviceLog()->critical ("Session monitor disabled");
                break;
            }
            sd_login_monitor_flush (this->impl_->monitor);
            this->poll ();
        }

        this->impl_->activeXSession.reset();
        this->impl_->activeUser.reset();
        this->impl_->activeXDisplay.reset();
    });

    this->impl_->stopped = false;
}

void
SessionMonitor::stop () {
    if (impl_->stopped) {
        return;
    }
    impl_->fd.cancel ();
    ioService_.poll ();
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
                serviceLog()->error ("Couldn't determine type of active login "
                                     "session: {}", session);
                return;
            }
            if (strcmp (type, "x11") != 0) {
                serviceLog()->warn ("Active session is not running an X server. "
                                    "Ignoring session: ", session);
                return;
            }

            char* sclass = nullptr;
            BOOST_SCOPE_EXIT (&sclass) {
                free_and_null (sclass);
            }
            BOOST_SCOPE_EXIT_END

            if (sd_session_get_class (session.c_str (), &sclass) < 0) {
                serviceLog()->critical ("Couldn't determine 'class' of active X "
                                        "session: {}. Ignoring", session);
                return;
            }

            if (!impl_->activeXSession || (session != *impl_->activeXSession)) {
                serviceLog()->info ("New active X session: {}, class: {}", session, sclass);
            }

            uid_t user;
            char* display = nullptr;
            BOOST_SCOPE_EXIT (&display) {
                free_and_null (display);
            }
            BOOST_SCOPE_EXIT_END


            if (sd_session_get_uid (session.c_str (), &user) < 0) {
                serviceLog()->critical ("Couldn't determine user of active X "
                                        "session: {}. Ignoring", session);
                return;
            }

            if (sd_session_get_display (session.c_str (), &display) < 0) {
                serviceLog()->critical ("Couldn't determine active X"
                                        "display: {}. Ignoring",
                                        session);
                return;
            }

            /* TODO: avoid restarting core twice if *both* the display and
             * active user change.
             */
            if (!impl_->activeUser || (*impl_->activeUser != user)) {
                serviceLog()->info ("New active X user: {} -> {}",
                                    impl_->activeUser, user);
                if (user < 1000) {
                    serviceLog()->warn ("Ignoring X session switch to system user {}", user);
                    continue;
                }
                impl_->activeUser = user;
                activeUserChanged (std::to_string (user));
            }

            if (!impl_->activeXDisplay || (*impl_->activeXDisplay != display)) {
                serviceLog()->info ("New active X display: {} -> {}",
                                    impl_->activeXDisplay, display);
                impl_->activeXDisplay = display;
                activeDisplayChanged (display);
            }

            return;
        }
    }
}

