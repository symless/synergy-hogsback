cmake_minimum_required (VERSION 3.4)

set (SYNERGY_BUILD_LEGACY_GUI OFF CACHE BOOL "Synergy: Build GUI v1" FORCE)
set (SYNERGY_BUILD_LEGACY_SERVICE OFF CACHE BOOL "Synergy: Build the v1 service (synergyd)" FORCE)
set (SYNERGY_BUILD_LEGACY_INSTALLER OFF CACHE BOOL "Synergy: Configure the v1 installer" FORCE)

add_subdirectory (core)
add_subdirectory (gui)
add_subdirectory (service)
add_subdirectory (common)
add_subdirectory (tests)
