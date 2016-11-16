project (synergy CXX)
cmake_minimum_required (VERSION 3.4)

# Find includes in corresponding build directories
set(CMAKE_INCLUDE_CURRENT_DIR ON)

# Instruct CMake to run moc automatically when needed.
set(CMAKE_AUTOMOC ON)

find_package(Qt5 COMPONENTS Widgets Quick Core)
include_directories (${Qt5Widgets_INCLUDE_DIRS} 
	             ${Qt5Qml_INCLUDE_DIRS}
		     ${Qt5Quick_INCLUDE_DIRS})

add_subdirectory (lib)
qt5_add_resources(RESOURCES qml.qrc)

add_executable (${PROJECT_NAME}
    main.cpp
    ${RESOURCES}
)

target_include_directories (${PROJECT_NAME} SYSTEM PUBLIC
    lib
    ${fmt_INCLUDE_DIR}
    ${Boost_INCLUDE_DIRS}
)

target_link_libraries (${PROJECT_NAME}
    gui_lib
    Qt5::Widgets
    Qt5::Qml
    Qt5::Quick
    fmt
    libzmq-static
    ${Boost_LIBRARIES}
    ${CMAKE_THREAD_LIBS_INIT}
)

if (WIN32)
    target_link_libraries (${PROJECT_NAME} ws2_32 rpcrt4 iphlpapi)
endif()
