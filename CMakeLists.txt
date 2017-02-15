cmake_minimum_required (VERSION 3.4)

set (CMAKE_CXX_STANDARD 14)
set (CMAKE_CXX_EXTENSIONS OFF)
set (CMAKE_CXX_STANDARD_REQUIRED ON)


set (CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH}
                       "${CMAKE_SOURCE_DIR}/cmake/Modules/")
set (CMAKE_RUNTIME_OUTPUT_DIRECTORY "${CMAKE_CURRENT_BINARY_DIR}/install/bin")
set (CMAKE_LIBRARY_OUTPUT_DIRECTORY "${CMAKE_CURRENT_BINARY_DIR}/install/lib")


if (WIN32)
    # Configure for Windows Vista and above. Vista has extended support from
    # Microsoft until April 11, 2017
    # https://msdn.microsoft.com/en-us/library/windows/desktop/aa383745(v=vs.85).aspx
    add_definitions (/DNTDDI_VERSION=0x06000000)
    add_definitions (/D_WIN32_WINNT=0x0600)
    add_definitions (/DWINVER=0x0600)

    if (MSVC)
        #set (SYNERGY_WARNING_FLAGS /W4)
    endif()

elseif (${CMAKE_CXX_COMPILER_ID} MATCHES "Clang")
    set (SYNERGY_WARNING_FLAGS  -Weverything
                                -Wno-c++98-compat -Wno-c++98-compat-pedantic)

elseif (CMAKE_COMPILER_IS_GNUCXX)
    set (SYNERGY_WARNING_FLAGS -Wall -Wextra -Wpedantic
                               -Wconversion -Wsign-conversion -Wformat=2)
    set (SYNERGY_CXX_FLAGS_RELEASE "-Os -flto -fvisibility=hidden -DNDEBUG")

endif()


if (NOT DEFINED SCAN_BUILD)
    add_definitions (${SYNERGY_WARNING_FLAGS})
endif()


if ((DEFINED SYNERGY_CXX_FLAGS_RELEASE) AND (CMAKE_BUILD_TYPE EQUAL "Release"))
    set (CMAKE_CXX_FLAGS_RELEASE ${SYNERGY_CXX_FLAGS_RELEASE})
elseif ((DEFINED SYNERGY_CXX_FLAGS_DEBUG) AND (CMAKE_BUILD_TYPE EQUAL "Debug"))
    set (CMAKE_CXX_FLAGS_DEBUG ${SYNERGY_CXX_FLAGS_DEBUG})
endif()


set (Boost_USE_STATIC_LIBS ON)
set (Boost_USE_STATIC_RUNTIME OFF)
set (Boost_USE_MULTITHREADED ON)

find_package (Threads REQUIRED)
find_package (Boost 1.53 REQUIRED COMPONENTS system date_time)

add_definitions (-DZMQ_STATIC)
add_definitions (-DZMQ_BUILD_DRAFT_API)
#add_definitions (-DZMQ_USE_LIBSODIUM)
include_directories (BEFORE SYSTEM third_party/libzmq/include)
include_directories (BEFORE SYSTEM third_party/cppzmq)

include_directories (third_party/catch/include)
add_subdirectory (third_party)

include_directories (.)
add_subdirectory (synergy)
