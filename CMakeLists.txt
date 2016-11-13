cmake_minimum_required (VERSION 3.4)

set (CMAKE_CXX_STANDARD 14)
set (CMAKE_CXX_EXTENSIONS OFF)
set (CMAKE_CXX_STANDARD_REQUIRED ON)

if (CMAKE_COMPILER_IS_GNUCXX)
set (CMAKE_CXX_FLAGS_RELEASE "-Os -flto -fvisibility=hidden -DNDEBUG")
endif()

include_directories (BEFORE SYSTEM third_party/libzmq/include)
include_directories (BEFORE SYSTEM third_party/cppzmq)

set (Boost_USE_MULTITHREADED ON)
set (Boost_USE_STATIC_LIBS ON)
set (Boost_USE_STATIC_RUNTIME OFF)

find_package (Boost 1.58 REQUIRED COMPONENTS system date_time)
find_package (Threads REQUIRED)

add_subdirectory (third_party)
add_subdirectory (service)
add_subdirectory (gui)
