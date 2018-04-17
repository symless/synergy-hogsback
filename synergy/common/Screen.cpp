#include <synergy/common/Screen.h>
#include <synergy/common/ScreenSnapshot.h>
#include <algorithm>
#include <boost/algorithm/string/join.hpp>

const int kScreenWidth = 86;
const int kScreenHeight = 58;

Screen::Screen(ScreenID id) noexcept: m_id (id) {
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

std::string
Screen::ipList() const
{
    return m_ipList;
}

bool
Screen::ipList (std::set<boost::asio::ip::address> const& ipList) {
    std::vector<std::string> ipStringList;
    ipStringList.reserve (ipList.size());

    std::transform (begin(ipList), end(ipList),
                    std::back_inserter(ipStringList),
                    [](auto const& ip) { return ip.to_string(); });

    auto list = boost::algorithm::join (ipStringList, ",");
    auto const changed = (m_ipList != list);
    if (changed) {
        m_ipList = std::move (list);
    }
    return changed;
}

bool
Screen::active() const
{
    return m_active;
}

void
Screen::apply(ScreenSnapshot const& ss)
{
    m_id = ss.id;
    m_name = ss.name;
    m_active = ss.active;
    m_x = ss.x_pos;
    m_y = ss.y_pos;

    // TODO: use actual resolution
    m_width = kScreenWidth;
    m_height = kScreenHeight;

    m_status = stringToScreenStatus(ss.status);
    m_ipList = ss.ipList;

    m_errorCode = static_cast<ScreenError>(ss.error_code);
    m_errorMessage = ss.error_message;
}

ScreenError Screen::errorCode() const
{
    return m_errorCode;
}

void Screen::setErrorCode(const ScreenError &errorCode)
{
    m_errorCode = errorCode;
}

std::string Screen::errorMessage() const
{
    return m_errorMessage;
}

void Screen::setErrorMessage(const std::string &errorMessage)
{
    m_errorMessage = errorMessage;
}
