cmake_minimum_required (VERSION 3.4)

add_subdirectory (fmt EXCLUDE_FROM_ALL)

set (ENABLE_CURVE OFF CACHE BOOL "Enable ZMQ CURVE security" FORCE)
set (WITH_OPENPGM OFF CACHE BOOL "Build ZMQ with support for OpenPGM" FORCE)
if (APPLE)
  set (ZMQ_BUILD_FRAMEWORK OFF CACHE BOOL "Build ZMQ as OS X framework" FORCE)
endif()
add_subdirectory (libzmq EXCLUDE_FROM_ALL)

set (CAPNP_LITE ON CACHE BOOL "Compile Cap'n Proto in 'lite mode'" FORCE)
set (BUILD_TESTING OFF CACHE BOOL "Build Cap'n Proto unit tests" FORCE)

if (WIN32)
    add_definitions (-DEXTERNAL_CAPNP=1)
endif()

add_subdirectory (capnproto/c++ EXCLUDE_FROM_ALL)
