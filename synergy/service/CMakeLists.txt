project (synergyd CXX)
cmake_minimum_required (VERSION 3.4)

set (SERVICE_SOURCES
    main.cpp
    ProcessManager.cpp
    SessionManager.cpp
)

add_executable (${PROJECT_NAME} ${SERVICE_SOURCES})

target_include_directories (${PROJECT_NAME} SYSTEM PUBLIC
    ${fmt_INCLUDE_DIR}
    ${Boost_INCLUDE_DIRS}
    ../common
)

target_link_libraries (${PROJECT_NAME}
    commonlib
    fmt
    ${Boost_LIBRARIES}
)

if (${CMAKE_SYSTEM_NAME} MATCHES "Darwin")
    add_subdirectory (PreLoginAgent)
endif()
