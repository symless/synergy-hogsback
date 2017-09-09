#include <synergy/common/Screen.h>

Screen::Screen(ScreenID id) noexcept: m_id (id) {
}

Screen
Screen::fromJsonSnapshot(std::string const&)
{
    Screen screen;
    return screen;
}

ScreenID
Screen::id() const {
    return m_id;
}

void
Screen::id (ScreenID id) {
   m_id = {id};
}

std::string
Screen::name() const {
    return m_name;
}

void
Screen::name (std::string str) {
    m_name = std::move(str);
}

void
Screen::x (int64_t x) {
    m_x = {x};
}

int64_t
Screen::x() const noexcept {
    return m_x;
}

void
Screen::y(int64_t y) {
    m_y = {y};
}

int64_t
Screen::y() const noexcept {
    return m_y;
}

void
Screen::height(int64_t h) {
    m_height = {h};
}

int64_t
Screen::height() const noexcept {
    return m_height;
}

void
Screen::width(int64_t w) {
    m_width = {w};
}

int64_t
Screen::width() const noexcept {
    return m_width;
}

void
Screen::status (ScreenStatus status) {
    m_status = status;
}

ScreenStatus
Screen::status() const noexcept {
    return m_status;
}

void
Screen::apply(Snapshot const&) {

}
