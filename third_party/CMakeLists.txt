cmake_minimum_required (VERSION 3.4)

if (NOT DEFINED SCAN_BUILD)
    remove_definitions (${SYNERGY_WARNING_FLAGS})
endif()

add_subdirectory (fmt EXCLUDE_FROM_ALL)
add_subdirectory (norm EXCLUDE_FROM_ALL)


set (FLATBUFFERS_INSTALL OFF CACHE BOOL
                         "Enable flatbuffers install targets." FORCE)
set (FLATBUFFERS_BUILD_TESTS OFF CACHE BOOL
                             "Enable flatbuffers tests and samples." FORCE)
add_subdirectory (flatbuffers EXCLUDE_FROM_ALL)


if (APPLE)
    set (ZMQ_BUILD_FRAMEWORK OFF CACHE BOOL "Build ZMQ as an OS X framework" FORCE)
endif()

set (ENABLE_CURVE ON CACHE BOOL "Enable ZMQ CURVE security" FORCE)
set (WITH_LIBSODIUM ON CACHE BOOL "ZMQ: Use libsodium instead of tweetnacl" FORCE)
set (WITH_OPENPGM OFF CACHE BOOL "Build ZMQ with support for OpenPGM" FORCE)
add_subdirectory (libzmq EXCLUDE_FROM_ALL)
