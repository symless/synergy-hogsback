project (synergyd CXX)
cmake_minimum_required (VERSION 3.4)

set (SERVICE_SOURCES
    main.cpp
    Service.cpp
    IServiceController.cpp
    MSWindowsServiceController.cpp
    ProcessManager.cpp
    SessionManager.cpp
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
    add_subdirectory (PreLoginAgent)
endif()
