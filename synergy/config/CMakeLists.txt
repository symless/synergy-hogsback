project (synergy-config CXX)
cmake_minimum_required (VERSION 3.4)

set (CMAKE_INCLUDE_CURRENT_DIR ON)
set (CMAKE_AUTOMOC ON)
set (CMAKE_AUTORCC ON)
set (CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} -DQT_QML_DEBUG ")

find_package (Qt5 COMPONENTS Core Widgets Quick Network Svg)

if (APPLE)
    find_library (ApplicationServices_LIBRARY ApplicationServices)
    find_library (Foundation_LIBRARY Foundation)
    find_library (AppKit_LIBRARY AppKit)
    find_library (ServiceManagement_LIBRARY ServiceManagement)
    find_library (Security_LIBRARY Security)
endif()

add_subdirectory (lib)
add_subdirectory (tests)

set(icon_resources)

if (WIN32)
    set (SYNERGY_WIN_RESOURCES
        res/icon/synergy.ico
        res/synergy.rc
    )
elseif (${CMAKE_SYSTEM_NAME} MATCHES "Darwin")
    set (SYNERGY_OSX_HELPER_SOURCES
        helper.mm
    )
endif()


add_executable (${PROJECT_NAME} WIN32
    main.cpp
    qml.qrc
    ${SYNERGY_WIN_RESOURCES}
    ${SYNERGY_OSX_HELPER_SOURCES}
    main.qml
    res/view/ActivationPage.qml
    res/view/AccessibilityPage.qml
    res/view/ConfigurationPage.qml
    res/view/ProfilePage.qml
    res/component/CustomBorder.qml
    res/component/CustomCheckBox.qml
    res/component/BodyText.qml
    res/component/HeaderText.qml
    res/component/LogoText.qml
    res/component/ProfileMenu.qml
    res/component/ProfileMenuButton.qml
    res/component/Version.qml
    res/component/SynergyButton.qml
    res/component/SynergyButtonStyle.qml
)

target_include_directories (${PROJECT_NAME} SYSTEM PUBLIC
    lib
    ${Boost_INCLUDE_DIRS}
)

target_link_libraries (${PROJECT_NAME}
    guilib
    fmt
    libzmq-static
    ${SODIUM_LIBRARIES}
    ${Boost_LIBRARIES}
    ${CMAKE_THREAD_LIBS_INIT}
    ${ApplicationServices_LIBRARY}
    ${AppKit_LIBRARY}
    ${Foundation_LIBRARY}
    ${Security_LIBRARY}
    ${ServiceManagement_LIBRARY}
)

if (${CMAKE_SYSTEM_NAME} MATCHES "Darwin")
    add_subdirectory (helper)
    install (TARGETS ${PROJECT_NAME} DESTINATION ${SYNERGY_BUNDLE_BINARY_DIR})
    install (PROGRAMS "${CMAKE_SOURCE_DIR}/third_party/precompiled/crashpad/macos/crashpad_handler"
             DESTINATION ${SYNERGY_BUNDLE_BINARY_DIR})
elseif (${CMAKE_SYSTEM_NAME} MATCHES "Linux")
    install (TARGETS ${PROJECT_NAME} DESTINATION bin)
endif()
