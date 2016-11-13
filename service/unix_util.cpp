#include <boost/system/error_code.hpp>
#include <boost/system/system_error.hpp>
#include <cerrno>

__attribute__ ((noinline, noreturn)) void
throw_errno () {
    throw boost::system::system_error (errno,
                                       boost::system::system_category ());
}
