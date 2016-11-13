project (synergyd CXX)
cmake_minimum_required (VERSION 3.4)

add_executable (${PROJECT_NAME}
    service.cpp
    lsif_bsd.cpp
    unix_util.cpp
    multicast.cpp
)

target_include_directories (${PROJECT_NAME} SYSTEM PUBLIC
    ${fmt_INCLUDE_DIR}
    ${Boost_INCLUDE_DIRS}
)

target_link_libraries (${PROJECT_NAME}
    fmt
    libzmq-static
    ${Boost_LIBRARIES}
    ${CMAKE_THREAD_LIBS_INIT}
)

add_executable (lsif
    lsif_bsd.cpp
    unix_util.cpp
)

target_compile_definitions (lsif PUBLIC LSIF_APP)
target_include_directories (lsif SYSTEM PUBLIC ${Boost_INCLUDE_DIRS})
target_link_libraries (lsif ${Boost_LIBRARIES})
