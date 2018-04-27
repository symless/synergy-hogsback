#include <synergy/common/Profile.h>

#include <synergy/common/ProfileSnapshot.h>

#include <synergy/service/json.hpp>

int64_t Profile::id() const
{
    return m_id;
}

void Profile::setId(const int64_t &id)
{
    m_id = id;
}

int64_t Profile::version() const
{
    return m_version;
}

void Profile::setVersion(const int64_t &version)
{
    m_version = version;
}

int64_t Profile::server() const
{
    return m_server;
}

void Profile::setServer(const int64_t &server)
{
    m_server = server;
}

std::string Profile::name() const
{
    return m_name;
}
