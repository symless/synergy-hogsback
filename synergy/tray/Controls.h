#ifndef SYNERGY_TRAY_CONTROLS_H
#define SYNERGY_TRAY_CONTROLS_H

#include <memory>
#include <boost/signals2.hpp>
#include <spdlog/spdlog.h>

class TrayControlsImpl;

class TrayControls final {
public:
    TrayControls();
    ~TrayControls() noexcept;

    void connect();
    void pause();
    void resume();

    std::shared_ptr<spdlog::logger> log() const;

private:
    std::unique_ptr<TrayControlsImpl> m_impl;

public:
    boost::signals2::signal<void()> ready;
};

#endif // SYNERGY_TRAY_CONTROLS_H
