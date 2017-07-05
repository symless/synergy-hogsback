#ifndef COMMONCONFIGPARSER_H
#define COMMONCONFIGPARSER_H

#include <boost/variant/get.hpp>
#include <boost/variant/variant.hpp>
#include <cstdint>
#include <memory>
#include <string>

// TODO: Make this as submodule so the code can be shared between cloud and synergy-v2
class  ConfigParser
{
public:
    using value_type = boost::variant<std::string, int64_t, double, bool>;

    static ConfigParser parse_file (std::string const& path);
    static ConfigParser parse_memory (char const* buf, size_t size);
    static ConfigParser parse_c_str (char const* const str);
    ConfigParser get_section (char const*) const;

    template <typename T>
    T
    get_value (char const* const key) const {
        auto const ptr = get_value_ptr (key);
        return boost::get<T> (get_value_of (ptr));
    }

    template <typename T>
    value_type
    get_value_or (char const* const key, T&& init) const {
        auto const ptr = get_value_ptr (key);
        if (!ptr) {
            return value_type (std::forward<T> (init));
        }
        return get_value_of (ptr);
    }

private:
    ConfigParser ();
    std::shared_ptr<void const> get_value_ptr (char const* key) const noexcept;

    value_type
    get_value_of (std::shared_ptr<void const> const& value_ptr) const;

private:
    std::shared_ptr<void const> impl_;
};

#endif // COMMONCONFIGPARSER_H
