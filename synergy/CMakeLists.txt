cmake_minimum_required (VERSION 3.4)

set (SYNERGY_BUILD_LEGACY_SERVICE OFF CACHE BOOL "Build the legacy service (synergyd)")

add_subdirectory (protocol)

add_subdirectory (core)

add_subdirectory (gui)

add_subdirectory (service)
