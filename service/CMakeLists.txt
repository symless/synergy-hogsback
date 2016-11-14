project (synergyd CXX)
cmake_minimum_required (VERSION 3.4)

set (SERVICE_SOURCES
    service.cpp
    multicast.cpp
)

if (WIN32)
    set (LSIF_SOURCES lsif_windows.cpp)
else()
    set (LSIF_SOURCES lsif_unix.cpp unix_util.cpp)
endif()

list (APPEND SERVICE_SOURCES ${LSIF_SOURCES})
add_executable (${PROJECT_NAME} ${SERVICE_SOURCES})

target_include_directories (${PROJECT_NAME} SYSTEM PUBLIC
    ${fmt_INCLUDE_DIR}
    ${Boost_INCLUDE_DIRS}
)

target_link_libraries (${PROJECT_NAME}
    fmt
    capnp kj
    libzmq-static
    ${Boost_LIBRARIES}
    ${CMAKE_THREAD_LIBS_INIT}
)

if (WIN32)
    target_link_libraries (${PROJECT_NAME} ws2_32 rpcrt4 iphlpapi)
endif()

add_executable (lsif ${LSIF_SOURCES})
target_compile_definitions (lsif PUBLIC LSIF_APP)
target_include_directories (lsif SYSTEM PUBLIC ${Boost_INCLUDE_DIRS})
target_link_libraries (lsif ${Boost_LIBRARIES})
