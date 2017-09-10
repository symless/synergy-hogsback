#ifndef SYNERGY_COMMON_SCREEN_H
#define SYNERGY_COMMON_SCREEN_H

#include <synergy/common/ScreenStatus.h>
#include <cstdint>
#include <string>
#include <boost/signals2.hpp>

using ScreenID = int64_t;

class ScreenSnapshot;

class Screen final {
    friend class ProfileConfig;

public:
    Screen() = default;
    explicit Screen(ScreenID id) noexcept;

    void id (ScreenID);
    ScreenID id() const;

    void name (std::string);
    std::string name() const;

    void x (int64_t);
    void y (int64_t);
    int64_t x() const noexcept;
    int64_t y() const noexcept;

    void width (int64_t);
    void height (int64_t);
    int64_t width() const noexcept;
    int64_t height() const noexcept;

    void status(ScreenStatus);
    ScreenStatus status() const noexcept;

    std::string successfulTestIp() const;
    std::string failedTestIp() const;

    std::string ipList() const;

    bool active() const;

    void apply (const ScreenSnapshot &ss);

private:
    ScreenID        m_id      = 0;
    std::string     m_name;
    bool            m_active  = false;
    int64_t         m_x       = 0;
    int64_t         m_y       = 0;
    int64_t         m_width   = 0;
    int64_t         m_height  = 0;
    ScreenStatus    m_status  = ScreenStatus::kDisconnected;
    std::string     m_successfulTestIp;
    std::string     m_failedTestIp;
    std::string     m_ipList;
};

#endif
