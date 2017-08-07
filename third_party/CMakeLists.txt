cmake_minimum_required (VERSION 3.4)

# Remove additional compiler flags for third party libraries. 
# We don't care about their issues.

if (NOT DEFINED SYNERGY_SCAN_BUILD)
    remove_definitions (${SYNERGY_WARNING_FLAGS})
endif()

#
# fmt library
#
add_subdirectory (fmt EXCLUDE_FROM_ALL)

#
# ZeroMQ
#
set (ENABLE_CURVE ON CACHE BOOL "ZeroMQ: Enable CURVE security" FORCE)
set (WITH_LIBSODIUM ON CACHE BOOL "ZeroMQ: Use libsodium instead of tweetnacl" FORCE)
set (WITH_OPENPGM OFF CACHE BOOL "ZeroMQ: Build with support for OpenPGM" FORCE)
if (APPLE)
    set (ZMQ_BUILD_FRAMEWORK OFF CACHE BOOL "ZeroMQ: Build OS X framework" FORCE)
endif()

add_subdirectory (libzmq EXCLUDE_FROM_ALL)

#
# Bonefish (WAMP router)
#
add_subdirectory (bonefish EXCLUDE_FROM_ALL)

#
# Msgpack
#
add_subdirectory (msgpack-c EXCLUDE_FROM_ALL)
