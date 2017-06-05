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

set(icon_resources)

if (WIN32)
set (icon_resources
    res/icon/synergy.ico
    res/synergy.rc
)
endif()

add_executable (${PROJECT_NAME} WIN32
    main.cpp
    qml.qrc
    ${icon_resources}
    main.qml
    ActivationPage.qml
    AccessibilityPage.qml
    ConfigurationPage.qml
    CustomBorder.qml
    CustomCheckBox.qml
    BodyText.qml
    HeaderText.qml
    LogoText.qml
    ProfilePage.qml
    ProfileMenu.qml
    ProfileMenuButton.qml
    Version.qml
    SynergyButton.qml
    SynergyButtonStyle.qml
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

if (CMAKE_BUILD_TYPE STREQUAL "RelWithDebInfo")
    if (WIN32)
        target_link_libraries (${PROJECT_NAME} ws2_32 rpcrt4 iphlpapi)
        target_link_libraries (${PROJECT_NAME}
            "${CMAKE_SOURCE_DIR}/third_party/precompiled/crashpad/windows/crashpad_client.lib"
            "${CMAKE_SOURCE_DIR}/third_party/precompiled/crashpad/windows/crashpad_util.lib"
            "${CMAKE_SOURCE_DIR}/third_party/precompiled/crashpad/windows/base.lib")
    elseif (${CMAKE_SYSTEM_NAME} MATCHES "Darwin")
        find_library (IOKit_LIBRARY IOKit)
        find_library (Security_LIBRARY Security)

        target_link_libraries (${PROJECT_NAME}
            "${CMAKE_SOURCE_DIR}/third_party/precompiled/crashpad/macos/libcrashpad_client.a"
            "${CMAKE_SOURCE_DIR}/third_party/precompiled/crashpad/macos/libcrashpad_util.a"
            "${CMAKE_SOURCE_DIR}/third_party/precompiled/crashpad/macos/libbase.a"
            ${Security_LIBRARY}
            ${IOKit_LIBRARY}
            "/usr/lib/libbsm.0.dylib")
    endif()
endif()

if (${CMAKE_SYSTEM_NAME} MATCHES "Darwin")
    install (TARGETS ${PROJECT_NAME} DESTINATION ${SYNERGY_BUNDLE_BINARY_DIR})
    install (PROGRAMS "${CMAKE_SOURCE_DIR}/third_party/precompiled/crashpad/macos/crashpad_handler"
             DESTINATION  ${SYNERGY_BUNDLE_BINARY_DIR})
elseif (${CMAKE_SYSTEM_NAME} MATCHES "Linux")
    install (TARGETS ${PROJECT_NAME} DESTINATION bin)
endif()

