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
    ${platform_sources}
)

if (${CMAKE_SYSTEM_NAME} MATCHES "Windows")
    set (OPENSSL_ROOT "${CMAKE_SOURCE_DIR}/synergy/core/ext/openssl/windows")
    if (CMAKE_SIZEOF_VOID_P EQUAL 8)
        set (ENV{OPENSSL_ROOT_DIR} "${OPENSSL_ROOT}/x64")
    else()
        set (ENV{OPENSSL_ROOT_DIR} "${OPENSSL_ROOT}/x86")
    endif()
    include_directories (BEFORE SYSTEM "$ENV{OPENSSL_ROOT_DIR}/include")
endif()

find_package (OpenSSL REQUIRED)

set (Autobahn_INCLUDE_DIR "${CMAKE_SOURCE_DIR}/third_party/autobahn-cpp")
set (Bonefish_INCLUDE_DIR "${CMAKE_SOURCE_DIR}/third_party/bonefish/src")
set (Websocketpp_INCLUDE_DIR "${CMAKE_SOURCE_DIR}/third_party/websocketpp")
set (Cpptoml_INCLUDE_DIR "${CMAKE_SOURCE_DIR}/third_party/cpptoml/include")

include_directories (
    ${Boost_INCLUDE_DIRS}
    ${Autobahn_INCLUDE_DIR}
    ${Bonefish_INCLUDE_DIR}
    ${Websocketpp_INCLUDE_DIR}
    ${Cpptoml_INCLUDE_DIR}
    ${OPENSSL_INCLUDE_DIR}
)

add_library (commonlib ${COMMON_LIB_SOURCES})
target_link_libraries(commonlib bonefish)
