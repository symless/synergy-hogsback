project (synergy-tray CXX)
cmake_minimum_required (VERSION 3.4)

set (CMAKE_INCLUDE_CURRENT_DIR ON)
set (CMAKE_AUTOMOC ON)
set (CMAKE_AUTORCC ON)

find_package (Qt5 COMPONENTS Core Widgets)

add_executable (${PROJECT_NAME}
    main.cpp
    Controls.cpp
    Tray.cpp
    tray.qrc
)

target_link_libraries (${PROJECT_NAME}
    commonlib
    Qt5::Core
    Qt5::Widgets
    ${CMAKE_THREAD_LIBS_INIT}
)

if (${CMAKE_SYSTEM_NAME} MATCHES "Darwin")
    install (TARGETS ${PROJECT_NAME} DESTINATION ${SYNERGY_BUNDLE_BINARY_DIR})
elseif (${CMAKE_SYSTEM_NAME} MATCHES "Linux")
    install (TARGETS ${PROJECT_NAME} DESTINATION bin)
endif()
