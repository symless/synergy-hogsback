#include "ProfileSnapshot.h"
#include "json.hpp"

BOOST_FUSION_ADAPT_STRUCT(
    ProfileSnapshot::Profile,
    (int, configVersion)
    (int, id)
    (std::string, name)
    (int, server)
)

BOOST_FUSION_ADAPT_STRUCT(
    ProfileSnapshot::Screen,
    (int, id)
    (std::string, name)
    (bool, active)
    (std::string, ipList)
    (std::string, status)
    (int, x_pos)
    (int, y_pos)
)

BOOST_FUSION_ADAPT_STRUCT(
    ProfileSnapshot::Snapshot,
    (ProfileSnapshot::Profile, profile)
    (std::vector<ProfileSnapshot::Screen>, screens)
)

void ProfileSnapshot::parseJsonSnapshot(std::string const &jsonConfig)
{
    from_json(m_data, jsonConfig);
}

const std::vector<ProfileSnapshot::Screen>& ProfileSnapshot::getScreens() const
{
    return m_data.screens;
}
