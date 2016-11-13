project (synergy CXX)
cmake_minimum_required (VERSION 3.4)

add_executable (${PROJECT_NAME}
    main.cpp
)

target_include_directories (${PROJECT_NAME} BEFORE PUBLIC .)
target_include_directories (${PROJECT_NAME} PUBLIC
    libzmq-static
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