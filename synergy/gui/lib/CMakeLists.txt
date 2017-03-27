cmake_minimum_required (VERSION 3.4)

set (GUI_LIB_SOURCES
    AppConfig.cpp
    Common.cpp
    CloudClient.cpp
    ConfigFileManager.cpp
    ConnectivityTester.cpp
    CoreInterface.cpp
    DebugLevel.cpp
    DirectoryManager.cpp
    Hostname.cpp
    IpManager.cpp
    LogManager.cpp
    ProcessCommand.cpp
    ProcessManager.cpp
    ScreenBBArrangement.cpp
    Screen.cpp
    ScreenListModel.cpp
    ScreenListSnapshotManager.cpp
    ScreenManager.cpp
    ServerInfo.cpp
    TestDelegatee.cpp
)

add_library (guilib ${GUI_LIB_SOURCES})
target_compile_definitions (guilib PRIVATE -DSYNLIB)
qt5_use_modules (guilib Core Widgets Quick Network)
