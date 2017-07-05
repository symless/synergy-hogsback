cmake_minimum_required (VERSION 3.4)

if (WIN32)
    file(GLOB platform_sources "MSWindows*.cpp")
elseif (APPLE)
    file(GLOB platform_sources "OSX*.cpp")
elseif (UNIX)
    file(GLOB platform_sources "XWindows*.cpp")
endif()

set (COMMON_LIB_SOURCES
    GlobalConfig.cpp
    UserConfig.cpp
    DirectoryManager.cpp
    ${platform_sources}
)

include_directories (
    ${Boost_INCLUDE_DIRS}
)

add_library (commonlib ${COMMON_LIB_SOURCES})
