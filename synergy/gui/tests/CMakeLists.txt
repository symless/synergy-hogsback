cmake_minimum_required (VERSION 3.4)

find_package (Qt5 COMPONENTS Test REQUIRED)

set (GUI_TEST_SOURCES
    main.cpp
    ScreenBBArrangementTest.cpp
    AutoTest.h
    mock/MockDirectoryManager.cpp
    mock/MockScreenModel.cpp
)

add_executable (gui-tests ${GUI_TEST_SOURCES})

include_directories(
    ../lib
    mock
)

qt5_use_modules (gui-tests Test)

target_link_libraries (gui-tests guilib
    ${ApplicationServices_LIBRARY}
    ${Foundation_LIBRARY}
    ${AppKit_LIBRARY}
    commonlib
    ${Boost_LIBRARIES}
    ${CMAKE_THREAD_LIBS_INIT}
)
