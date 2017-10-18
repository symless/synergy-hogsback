cmake_minimum_required (VERSION 3.4)

if (${CMAKE_SYSTEM_NAME} MATCHES "Darwin")
    set (GUI_LIB_MACOS_SOURCES Accessibility.mm)
endif()

set (GUI_LIB_SOURCES
    App.cpp
    AppConfig.cpp
    AccessibilityManager.cpp
    Common.cpp
    CloudClient.cpp
    DebugLevelUtil.cpp
    DirectoryManager.cpp
    FontManager.cpp
    Hostname.cpp
    IpManager.cpp
    LogManager.cpp
    Profile.cpp
    ProfileListModel.cpp
    ServiceProxy.cpp
    ScreenBBArrangement.cpp
    UIScreen.cpp
    ScreenListModel.cpp
    ScreenListSnapshotManager.cpp
    ScreenManager.cpp
    Profile.cpp
    ProfileListModel.cpp
    ProfileManager.cpp
    VersionManager.cpp
    ErrorView.cpp
    ${GUI_LIB_MACOS_SOURCES}
)

add_library (guilib ${GUI_LIB_SOURCES})
target_compile_definitions (guilib PRIVATE -DSYNLIB})

target_link_libraries(guilib commonlib)

qt5_use_modules (guilib Core Widgets Quick Network Svg)
