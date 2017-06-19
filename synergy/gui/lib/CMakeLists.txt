cmake_minimum_required (VERSION 3.4)

if (${CMAKE_SYSTEM_NAME} MATCHES "Darwin")
    set (GUI_LIB_MACOS_SOURCES Accessibility.mm)
endif()

set (GUI_LIB_SOURCES
    AppConfig.cpp
    AccessibilityManager.cpp
    Common.cpp
    CloudClient.cpp
    ConfigFileManager.cpp
    ConnectivityTester.cpp
    CoreInterface.cpp
    DebugLevel.cpp
    DirectoryManager.cpp
    FontManager.cpp
    Hostname.cpp
    IpManager.cpp
    LogManager.cpp
    Profile.cpp
    ProfileListModel.cpp
    ProcessCommand.cpp
    ProcessManager.cpp
    ScreenBBArrangement.cpp
    Screen.cpp
    ScreenListModel.cpp
    ScreenListSnapshotManager.cpp
    ScreenManager.cpp
    TrialValidator.cpp
    Profile.cpp
    ProfileListModel.cpp
    ProfileManager.cpp
    TestDelegatee.cpp
    VersionManager.cpp
    ${GUI_LIB_MACOS_SOURCES}
)

add_library (guilib ${GUI_LIB_SOURCES})
target_compile_definitions (guilib PRIVATE -DSYNLIB})

if (SYNERGY_DEVELOPER_MODE)
    target_compile_definitions(guilib PRIVATE -DSYNERGY_DEVELOPER_MODE=1)
endif()

qt5_use_modules (guilib Core Widgets Quick Network Svg)
