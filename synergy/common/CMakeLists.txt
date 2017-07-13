cmake_minimum_required (VERSION 3.4)

if (WIN32)
    file(GLOB platform_sources "MSWindows*.cpp")
elseif (APPLE)
    file(GLOB platform_sources "OSX*.cpp")
elseif (UNIX)
    file(GLOB platform_sources "XWindows*.cpp")
endif()

set (COMMON_LIB_SOURCES
    ConfigParser.cpp
    GlobalConfig.cpp
    UserConfig.cpp
    DirectoryManager.cpp
    RpcManager.cpp
    WampServer.cpp
    WampClient.cpp
    WampRouter.cpp
    RpcClient.cpp
    ${platform_sources}
)

set (Bonefish_INCLUDE_DIR "${CMAKE_SOURCE_DIR}/third_party/bonefish/src")
set (Cpptoml_INCLUDE_DIR "${CMAKE_SOURCE_DIR}/third_party/cpptoml/include")

include_directories (
    ${Bonefish_INCLUDE_DIR}
    ${Cpptoml_INCLUDE_DIR}
)

add_library (commonlib ${COMMON_LIB_SOURCES})
target_link_libraries(commonlib bonefish)
