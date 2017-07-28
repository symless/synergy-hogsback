project (synergyd CXX)
cmake_minimum_required (VERSION 3.4)

if (WIN32)
    file(GLOB platform_sources "Windows/ServiceController.cpp")
else ()
    file(GLOB platform_sources "Unix/ServiceController.cpp")
endif()

set (SERVICE_SOURCES
    main.cpp
    ProcessManager.cpp
    SessionManager.cpp
    ServiceWorker.cpp
    ${platform_sources}
)

add_executable (${PROJECT_NAME} ${SERVICE_SOURCES})

target_include_directories (${PROJECT_NAME} SYSTEM PUBLIC
    ${fmt_INCLUDE_DIR}
    ${Boost_INCLUDE_DIRS}
)

target_compile_definitions (${PROJECT_NAME}
    PRIVATE -DBOOST_COROUTINES_NO_DEPRECATION_WARNING
)

target_link_libraries (${PROJECT_NAME}
    fmt
    commonlib
    ${Boost_LIBRARIES}
    ${CMAKE_THREAD_LIBS_INIT}
)

if (${CMAKE_SYSTEM_NAME} MATCHES "Darwin")
    install (TARGETS ${PROJECT_NAME} DESTINATION ${SYNERGY_BUNDLE_BINARY_DIR})
elseif (${CMAKE_SYSTEM_NAME} MATCHES "Linux")
    install(FILES ${CMAKE_SOURCE_DIR}/dist/linux/synergy.service DESTINATION lib/systemd/system)
endif()
