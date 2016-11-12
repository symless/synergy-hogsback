project(synergyd CXX)
cmake_minimum_required (VERSION 3.4)

set (CXX_STANDARD 14)
set (CXX_EXTENIONS OFF)
set (CXX_STANDARD_REQUIRED ON)

if (CMAKE_COMPILER_IS_GNUCXX)
set (CMAKE_CXX_FLAGS_RELEASE "-O3 -flto -fvisibility=hidden -DNDEBUG")
endif()

find_package (Threads REQUIRED)
find_library (ZMQ_LIB zmq REQUIRED)

set (Boost_USE_MULTITHREADED ON)
set (Boost_USE_STATIC_LIBS ON)
set (Boost_USE_STATIC_RUNTIME OFF)
find_package (Boost 1.61 REQUIRED COMPONENTS system date_time)

set (fmt_INCLUDE_DIR ./ext/fmt)
aux_source_directory (./ext/fmt/fmt FMT_SOURCES)
add_library (fmt STATIC ${FMT_SOURCES})

add_executable (${PROJECT_NAME} service.cpp lsif.cpp multicast.cpp)
target_include_directories (${PROJECT_NAME} BEFORE PUBLIC .)
target_include_directories (${PROJECT_NAME} PUBLIC ${fmt_INCLUDE_DIR}
                                                   ${Boost_INCLUDE_DIRS})
target_link_libraries (${PROJECT_NAME} ${ZMQ_LIB} fmt ${Boost_LIBRARIES}
                       ${CMAKE_THREAD_LIBS_INIT})

add_executable (stub_gui stub_gui.cpp)
target_include_directories (stub_gui BEFORE PUBLIC .)
target_include_directories (stub_gui PUBLIC ${fmt_INCLUDE_DIR}
                                            ${Boost_INCLUDE_DIRS})
target_link_libraries (stub_gui ${ZMQ_LIB} fmt ${Boost_LIBRARIES}
                                 ${CMAKE_THREAD_LIBS_INIT})

add_executable (lsif lsif.cpp)
target_compile_definitions (lsif PUBLIC LSIF_APP)
target_include_directories (lsif PUBLIC ${Boost_INCLUDE_DIRS})
target_link_libraries (lsif ${Boost_LIBRARIES})
