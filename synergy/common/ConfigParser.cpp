#include "ConfigParser.h"

#include <boost/iostreams/stream.hpp>
#include <cpptoml.h>

namespace {

template <typename T, typename... Types>
struct value_extractor;

template <typename T>
struct value_extractor<T> {
    bool
    operator() (cpptoml::base const& base, ConfigParser::value_type& value) const {
        if (auto val = base.as<T> ()) {
            value = val->get ();
            return true;
        }
        return false;
    }
};

template <typename... Types>
struct value_extractor<boost::detail::variant::void_, Types...> {
	bool
	operator() (cpptoml::base const& base, ConfigParser::value_type& value) const {
		return false;
	}
};

template <typename T, typename... Types>
struct value_extractor {
    bool
    operator() (cpptoml::base const& base, ConfigParser::value_type& value) const {
        if (auto val = base.as<T> ()) {
            value = val->get ();
            return true;
        }
        value_extractor<Types...> extract;
        return extract (base, value);
    }
};

template <typename... Types>
inline bool
extract_value (cpptoml::base const& base, boost::variant<Types...>& value) {
    value_extractor<Types...> extract;
    return extract (base, value);
}

} // namespace

ConfigParser::ConfigParser () {
}

ConfigParser
ConfigParser::parse_file (std::string const& path) {
    ConfigParser config;
    config.impl_ = cpptoml::parse_file (path);
    return config;
}

ConfigParser
ConfigParser::parse_memory (const char* const ptr, size_t const size) {
    ConfigParser config;
    boost::iostreams::stream_buffer<boost::iostreams::array_source> sb (ptr,
                                                                        size);
    std::istream is (&sb);
    is.imbue (std::locale::classic ());
    cpptoml::parser parser (is);
    config.impl_ = parser.parse ();
    return config;
}

ConfigParser
ConfigParser::parse_c_str (const char* const str) {
    return parse_memory (str, std::strlen (str));
}

bool ConfigParser::isValid() const
{
    return impl_ ? true : false;
}

ConfigParser
ConfigParser::get_section (const char* const key) const {
    ConfigParser config;
    auto table   = std::static_pointer_cast<cpptoml::table const> (impl_);
    config.impl_ = table->get_table_qualified (key);
    return config;
}

std::shared_ptr<void const>
ConfigParser::get_value_ptr (char const* const key) const noexcept {
    std::shared_ptr<void const> ptr;
    auto table = std::static_pointer_cast<cpptoml::table const> (impl_);
    try {
        auto base = table->get_qualified (key);
        if (base->is_value ()) {
            ptr = std::move (base);
        }
    } catch (...) {
    }
    return ptr;
}

ConfigParser::value_type
ConfigParser::get_value_of (std::shared_ptr<void const> const& value_ptr) const {
    if (!value_ptr) {
        throw;
    }
    auto base = std::static_pointer_cast<cpptoml::base const> (value_ptr);
    value_type value;
    if (!extract_value (*base, value)) {
        throw;
    }
    return value;
}
