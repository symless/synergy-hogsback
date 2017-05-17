cmake_minimum_required (VERSION 3.4)

set (SYNERGY_BUILD_LEGACY_GUI OFF CACHE BOOL "Synergy: Build the legacy GUI" FORCE)
set (SYNERGY_BUILD_LEGACY_SERVICE OFF CACHE BOOL "Synergy: Build the legacy service (synergyd)" FORCE)
set (SYNERGY_BUILD_LEGACY_INSTALLER OFF CACHE BOOL "Synergy: Configure the legacy (v1) installer" FORCE)

add_subdirectory (protocol)

add_subdirectory (core)

add_subdirectory (gui)

add_subdirectory (service)
