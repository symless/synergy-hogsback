project (synsim CXX)
cmake_minimum_required (VERSION 3.4)

set (CMAKE_INCLUDE_CURRENT_DIR ON)
set (CMAKE_AUTOMOC ON)
set (CMAKE_AUTORCC ON)
set (CMAKE_AUTOUIC ON)

find_package (Qt5 COMPONENTS Widgets Core Network)

add_executable (${PROJECT_NAME}
    main.cpp
    MainWindow.cpp
    MainWindow.h
    MainWindow.ui
)

include_directories (../lib)
target_link_libraries (${PROJECT_NAME} synlib)
qt5_use_modules (${PROJECT_NAME} Widgets Core Network)
