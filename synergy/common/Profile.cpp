#include <synergy/common/Profile.h>

Profile::Profile (int64_t const id): m_id (id) {
}

const std::vector<Screen>&
Profile::getScreens() const
{
    return m_screens;
}

void
Profile::apply (Snapshot const&)
{
}
