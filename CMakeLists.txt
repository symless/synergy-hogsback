project(synergyd CXX)
cmake_minimum_required (VERSION 3.4)

set (CXX_STANDARD 14)
set (CXX_EXTENIONS OFF)
set (CXX_STANDARD_REQUIRED ON)

set (CMAKE_CXX_FLAGS_RELEASE "-O2 -flto -fvisibility=hidden -DNDEBUG")

find_package (Threads REQUIRED)
find_library (ZMQ_LIB zmq REQUIRED)
find_package (Boost 1.62 REQUIRED COMPONENTS system)

aux_source_directory (./fmt/fmt FMT_SOURCES)
add_library (fmt STATIC ${FMT_SOURCES})

#aux_source_directory (. SRC_LIST)

add_executable (${PROJECT_NAME} service.cpp)
target_include_directories (${PROJECT_NAME} BEFORE PUBLIC .)
target_include_directories (${PROJECT_NAME} PUBLIC ./fmt)
target_link_libraries (${PROJECT_NAME} ${ZMQ_LIB} fmt ${CMAKE_THREAD_LIBS_INIT})

add_executable (lsif lsif.cpp)
target_include_directories (lsif PUBLIC ./fmt ${Boost_INCLUDE_DIRS})
target_link_libraries (lsif fmt ${Boost_LIBRARIES})
