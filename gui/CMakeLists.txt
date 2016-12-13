project (synergy2 CXX)
cmake_minimum_required (VERSION 3.4)

set (CMAKE_INCLUDE_CURRENT_DIR ON)
set (CMAKE_AUTOMOC ON)
set (CMAKE_AUTORCC ON)

find_package(Qt5 COMPONENTS Widgets Quick Core)
add_subdirectory (lib)

add_executable (${PROJECT_NAME}
    main.cpp
    qml.qrc
    main.qml
    AdvancedPage.qml
    ConfigSettingView.qml
    ConfigurationPage.qml
    CustomBorder.qml
    CustomCheckBox.qml
    GeneralSettingView.qml
    LogSettingView.qml
)

target_include_directories (${PROJECT_NAME} SYSTEM PUBLIC
    lib
    ${fmt_INCLUDE_DIR}
    ${Boost_INCLUDE_DIRS}
)

target_link_libraries (${PROJECT_NAME}
    synlib
    fmt
    libzmq-static
    ${Boost_LIBRARIES}
    ${CMAKE_THREAD_LIBS_INIT}
)

if (WIN32)
    target_link_libraries (${PROJECT_NAME} ws2_32 rpcrt4 iphlpapi)
endif()
