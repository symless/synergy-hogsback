#ifndef SYNERGY_COMMON_SCREEN_H
#define SYNERGY_COMMON_SCREEN_H

#include <synergy/common/ErrorMessage.h>
#include <synergy/common/ScreenStatus.h>
#include <cstdint>
#include <string>
#include <boost/signals2.hpp>

using ScreenID = int64_t;

class Screen final {
public:
    template <typename... Args>
    using signal = boost::signals2::signal<Args...>;

    explicit
    Screen(int64_t id) noexcept: m_id (id) {
    }

    ScreenID
    id() const {
        return m_id;
    }

    void
    id (ScreenID id) {
       m_id = {id};
    }

    std::string
    name() const {
        return m_name;
    }

    void
    name (std::string str) {
        m_name = std::move(str);
    }

    int64_t
    x() const noexcept {
        return m_x;
    }

    void
    x (int64_t x) noexcept {
        m_x = {x};
    }

    int64_t
    y() const noexcept {
        return m_y;
    }

    void
    y(int64_t y) noexcept {
        m_y = {y};
    }

    int64_t
    height() const noexcept {
        return m_height;
    }

    void
    height(int64_t h) noexcept {
        m_height = {h};
    }

    int64_t
    width() const noexcept {
        return m_width;
    }

    void
    width(int64_t w) noexcept {
        m_width = {w};
    }

    ScreenStatus
    status() const noexcept {
        return m_status;
    }

private:
    ScreenID    m_id      = 0;
    std::string m_name;
    int64_t     m_x       = 0;
    int64_t     m_y       = 0;
    int64_t     m_width   = 0;
    int64_t     m_height  = 0;
    ScreenStatus m_status = ScreenStatus::kDisconnected;

public:
   // signal<void(ScreenStatus)> onStatusChanged;
   // signal<void(ErrorCode)> onConnectionError;
};

#endif
