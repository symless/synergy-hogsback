project (synergy2 CXX)
cmake_minimum_required (VERSION 3.4)

set (CMAKE_INCLUDE_CURRENT_DIR ON)
set (CMAKE_AUTOMOC ON)
set (CMAKE_AUTORCC ON)
set (CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} -DQT_QML_DEBUG ")

find_package (Qt5 COMPONENTS Core Widgets Quick Network)

if (APPLE)
    find_library (ApplicationServices_LIBRARY ApplicationServices)
    find_library (Foundation_LIBRARY Foundation)
    find_library (AppKit_LIBRARY AppKit)
endif()

add_subdirectory (lib)
add_subdirectory (tests)
add_subdirectory (simulator)

add_executable (${PROJECT_NAME} WIN32
    main.cpp
    qml.qrc
    main.qml
    AdvancedPage.qml
    ActivationPage.qml
    AccessibilityPage.qml
    ConfigSettingView.qml
    ConfigurationPage.qml
    CustomBorder.qml
    CustomCheckBox.qml
    GeneralSettingView.qml
    LogSettingView.qml
    Version.qml
)

target_include_directories (${PROJECT_NAME} SYSTEM PUBLIC
    lib
    ${fmt_INCLUDE_DIR}
    ${Boost_INCLUDE_DIRS}
)

target_link_libraries (${PROJECT_NAME}
    guilib
    fmt
    libzmq-static
    ${SODIUM_LIBRARIES}
    ${Boost_LIBRARIES}
    ${CMAKE_THREAD_LIBS_INIT}
    ${APP_SERVICES_LIBRARY}
    ${ApplicationServices_LIBRARY}
    ${Foundation_LIBRARY}
    ${AppKit_LIBRARY}
)

if (WIN32)
    target_link_libraries (${PROJECT_NAME} ws2_32 rpcrt4 iphlpapi)
endif()

if (${CMAKE_SYSTEM_NAME} MATCHES "Darwin")
    install (TARGETS ${PROJECT_NAME} DESTINATION ${SYNERGY_BUNDLE_BINARY_DIR})
elseif (${CMAKE_SYSTEM_NAME} MATCHES "Linux")
    install (TARGETS ${PROJECT_NAME} DESTINATION bin)
endif()

