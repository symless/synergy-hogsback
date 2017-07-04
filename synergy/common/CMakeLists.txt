cmake_minimum_required (VERSION 3.4)

set (COMMON_LIB_SOURCES
    GlobalConfig.cpp
    UserConfig.cpp
    DirectoryManager.cpp
)

add_library (commonlib ${COMMON_LIB_SOURCES})
