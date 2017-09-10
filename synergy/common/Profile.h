#pragma once

#include <cstdint>
#include <string>

class Profile final {
    friend class ProfileConfig;

public: 
    int64_t id() const;
    void setId(const int64_t &id);
    int64_t version() const;
    void setVersion(const int64_t &version);
    int64_t server() const;
    void setServer(const int64_t &server);
    std::string name() const;

private:
    Profile() = default;
    int64_t m_id = -1;
    int64_t m_version = -1;
    int64_t m_server = -1;
    std::string m_name;
};
