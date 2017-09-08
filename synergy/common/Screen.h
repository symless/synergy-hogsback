#ifndef SYNERGY_COMMON_SCREEN_H
#define SYNERGY_COMMON_SCREEN_H

#include <synergy/common/ScreenStatus.h>
#include <cstdint>
#include <string>
#include <boost/signals2.hpp>

using ScreenID = int64_t;

class Screen final {
public:
    template <typename... Args>
    using signal = boost::signals2::signal<Args...>;

    explicit Screen(ScreenID id) noexcept;
    ScreenID id() const;
    void id (ScreenID id);

    std::string name() const;
    void name (std::string);

    int64_t x() const noexcept;
    void x (int64_t);

    int64_t y() const noexcept;
    void y (int64_t);

    int64_t width() const noexcept;
    void width (int64_t);

    int64_t height() const noexcept;
    void height (int64_t);

    ScreenStatus status() const noexcept;

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
