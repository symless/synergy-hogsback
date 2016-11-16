cmake_minimum_required (VERSION 3.4)

add_subdirectory (fmt EXCLUDE_FROM_ALL)

set (ENABLE_CURVE OFF CACHE BOOL "Enable ZMQ CURVE security" FORCE)
set (WITH_OPENPGM OFF CACHE BOOL "Build ZMQ with support for OpenPGM" FORCE)
if (APPLE)
  set (ZMQ_BUILD_FRAMEWORK OFF CACHE BOOL "Build ZMQ as OS X framework" FORCE)
endif()
add_subdirectory (libzmq EXCLUDE_FROM_ALL)

if (WIN32)
    set (CAPNP_LITE ON CACHE BOOL "Compile Cap'n Proto in 'lite mode'" FORCE)
    add_definitions (-DEXTERNAL_CAPNP=1)
else()
    set (CAPNP_LITE OFF CACHE BOOL "Compile Cap'n Proto in 'lite mode'" FORCE)
endif()

set (BUILD_TESTING OFF CACHE BOOL "Build Cap'n Proto unit tests" FORCE)
add_subdirectory (capnproto/c++ EXCLUDE_FROM_ALL)

set (FLATBUFFERS_INSTALL OFF CACHE BOOL
        "Enable the installation of targets." FORCE)
set (FLATBUFFERS_BUILD_TESTS OFF CACHE BOOL
        "Enable build of flatbuffers tests and samples." FORCE)
add_subdirectory (flatbuffers EXCLUDE_FROM_ALL)

add_subdirectory (norm EXCLUDE_FROM_ALL)
