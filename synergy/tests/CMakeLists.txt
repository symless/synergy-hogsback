project (synergy-tests CXX)
cmake_minimum_required (VERSION 3.4)

file(GLOB_RECURSE TEST_SOURCES "*.cpp")

add_definitions (-DTEST_ENV)

# TODO: use lib instead of including sources
set(SERVICE_SOURCES
    ../service/WebsocketSession.cpp
    ../service/SecuredTcpSession.cpp
    ../service/SecuredTcpClient.cpp
    ../service/ServiceLogs.cpp
    ../service/CoreProcess.cpp
    ../service/CoreProcessImpl.cpp
)

add_executable (${PROJECT_NAME}
    ${TEST_SOURCES}
    ${SERVICE_SOURCES}
)

target_include_directories (${PROJECT_NAME} SYSTEM PUBLIC
    ${CMAKE_SOURCE_DIR}/third_party/catch/include
    ${CMAKE_SOURCE_DIR}/third_party/FakeIt/include
    ${CMAKE_SOURCE_DIR}/third_party/FakeIt/config/catch
    ${Boost_INCLUDE_DIRS}
)

target_compile_definitions (${PROJECT_NAME}
    PRIVATE -DBOOST_COROUTINES_NO_DEPRECATION_WARNING
)

# This is for fix crash when faking virtual destructor in FakeIt
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fno-devirtualize")

#
# OpenSSL
#
# TODO: this is duplicate from core
if (${CMAKE_SYSTEM_NAME} MATCHES "Windows")
    set (OPENSSL_ROOT ${CMAKE_SOURCE_DIR}/third_party/precompiled/openssl/windows)
    if (CMAKE_SIZEOF_VOID_P EQUAL 8)
        set (OPENSSL_ROOT "${OPENSSL_ROOT}/x64")
    else()
        set (OPENSSL_ROOT "${OPENSSL_ROOT}/x86")
    endif()

    include_directories (BEFORE SYSTEM ${OPENSSL_ROOT}/include)
    set (OPENSSL_LIBS
        ${OPENSSL_ROOT}/lib/libeay32.lib
        ${OPENSSL_ROOT}/lib/ssleay32.lib
    )
elseif (${CMAKE_SYSTEM_NAME} MATCHES "Darwin")
    set (OPENSSL_ROOT /usr/local/opt/openssl)

    include_directories (BEFORE SYSTEM ${OPENSSL_ROOT}/include)
    set (OPENSSL_LIBS
        ${OPENSSL_ROOT}/lib/libssl.a
        ${OPENSSL_ROOT}/lib/libcrypto.a
    )
elseif (${CMAKE_SYSTEM_NAME} MATCHES "Linux")
    set (OPENSSL_LIBS ssl crypto)
else()
    message (FATAL_ERROR "Couldn't find OpenSSL")
endif()

target_link_libraries (${PROJECT_NAME}
    commonlib
    ${Boost_LIBRARIES}
    ${CMAKE_THREAD_LIBS_INIT}
    ${OPENSSL_LIBS}
    fmt
)

if (${CMAKE_SYSTEM_NAME} MATCHES "Darwin")
    install (TARGETS ${PROJECT_NAME} DESTINATION ${SYNERGY_BUNDLE_BINARY_DIR})
elseif (${CMAKE_SYSTEM_NAME} MATCHES "Linux")
    install (TARGETS ${PROJECT_NAME} DESTINATION bin)
endif()
