project(synergyd CXX)
cmake_minimum_required (VERSION 3.4)

set (CXX_STANDARD 14)
set (CXX_EXTENIONS OFF)
set (CXX_STANDARD_REQUIRED ON)

#set (CMAKE_CXX_FLAGS_RELEASE "-O2 -flto -fvisibility=hidden -DNDEBUG")

find_package (Threads REQUIRED)
find_library (ZMQ_LIB zmq REQUIRED)

set (Boost_USE_MULTITHREADED ON)
set (Boost_USE_STATIC_LIBS ON)
set (Boost_USE_STATIC_RUNTIME OFF)
find_package (Boost 1.61 REQUIRED COMPONENTS system date_time)

aux_source_directory (./fmt/fmt FMT_SOURCES)
add_library (fmt STATIC ${FMT_SOURCES})

#add_definitions (-DDATE_TIME_NO_DEFAULT_CONSTRUCTOR)
#aux_source_directory (. SRC_LIST)

add_executable (${PROJECT_NAME} service.cpp lsif.cpp multicast.cpp)
target_include_directories (${PROJECT_NAME} BEFORE PUBLIC .)
target_include_directories (${PROJECT_NAME} PUBLIC ./fmt ${Boost_INCLUDE_DIRS})
target_link_libraries (${PROJECT_NAME} ${ZMQ_LIB} fmt ${Boost_LIBRARIES}
                       ${CMAKE_THREAD_LIBS_INIT})

add_executable (stub_gui stub_gui.cpp)
target_include_directories (stub_gui BEFORE PUBLIC .)
target_include_directories (stub_gui PUBLIC ./fmt ${Boost_INCLUDE_DIRS})
target_link_libraries (stub_gui ${ZMQ_LIB} fmt ${Boost_LIBRARIES}
                      ${CMAKE_THREAD_LIBS_INIT})

add_executable (lsif lsif.cpp)
target_compile_definitions (lsif PUBLIC LSIF_APP)
target_include_directories (lsif PUBLIC ${Boost_INCLUDE_DIRS})
target_link_libraries (lsif ${Boost_LIBRARIES})

