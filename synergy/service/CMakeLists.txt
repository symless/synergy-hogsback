project (synergy-service CXX)
cmake_minimum_required (VERSION 3.4)

if (${CMAKE_SYSTEM_NAME} MATCHES "Linux")
    set (SYSTEMD_LIBRARIES "systemd")
    set (SERVICE_SM_SOURCES
        "linux/SessionMonitor.cpp"
        "linux/ActiveXDisplay.cpp"
    )
else()
    set (SERVICE_SM_SOURCES "common/SessionMonitor.cpp")
endif()

if (WIN32)
    set (LSIF_SOURCES NetworkAdaptersWindows.cpp)
else()
    set (LSIF_SOURCES NetworkAdaptersUnix.cpp)
endif()

set (SERVICE_SOURCES
    main.cpp
    App.cpp
    CoreProcessImpl.cpp
    CoreProcess.cpp
    CoreManager.cpp
    ServiceWorker.cpp
    CloudClient.cpp
    SecuredTcpClient.cpp
    HttpSession.cpp
    WebsocketSession.cpp
    TerminationSignalListener.cpp
    ServiceLogs.cpp
    CoreStatusMonitor.cpp
    CoreErrorMonitor.cpp
    RouterErrorMonitor.cpp
    ErrorNotifier.cpp
    TrayService.cpp
    IPMonitor.cpp
    ProxyClient.cpp
    ${LSIF_SOURCES}
    ${SERVICE_SM_SOURCES}
)

add_executable (${PROJECT_NAME} ${SERVICE_SOURCES})

target_include_directories (${PROJECT_NAME} SYSTEM PUBLIC
                            ${Boost_INCLUDE_DIRS})

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

target_compile_definitions (${PROJECT_NAME}
    PRIVATE -DBOOST_COROUTINES_NO_DEPRECATION_WARNING
)

target_link_libraries (${PROJECT_NAME}
    commonlib
    router
    ${Boost_LIBRARIES}
    ${OPENSSL_LIBS}
    ${SYSTEMD_LIBRARIES}
    ${CMAKE_THREAD_LIBS_INIT}
)

if (WIN32)
    target_link_libraries (${PROJECT_NAME} iphlpapi)
endif()

add_subdirectory (router)

if (WIN32)
    add_subdirectory (windows)
endif()

if (${CMAKE_SYSTEM_NAME} MATCHES "Darwin")
    install (TARGETS ${PROJECT_NAME} DESTINATION ${SYNERGY_BUNDLE_BINARY_DIR})
elseif (${CMAKE_SYSTEM_NAME} MATCHES "Linux")
    install (TARGETS ${PROJECT_NAME} DESTINATION bin)
    install (FILES ${CMAKE_SOURCE_DIR}/debian/synergy.service
             DESTINATION lib/systemd/system)
endif()

add_executable (lsif ${LSIF_SOURCES})
target_compile_definitions (lsif PUBLIC LSIF_APP)
target_include_directories (lsif SYSTEM PUBLIC ${Boost_INCLUDE_DIRS})
target_link_libraries (lsif ${Boost_LIBRARIES})

if (WIN32)
    target_link_libraries (lsif iphlpapi)
endif()
