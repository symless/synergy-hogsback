project (synergy-controller CXX)
cmake_minimum_required (VERSION 3.4)


set (LOADER_SOURCES
    main.cpp
    ServiceController.cpp
)

add_executable (${PROJECT_NAME} ${LOADER_SOURCES})

target_link_libraries (${PROJECT_NAME}
    Wtsapi32 Userenv
)
