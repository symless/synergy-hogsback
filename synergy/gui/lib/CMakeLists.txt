cmake_minimum_required (VERSION 3.4)

set (GUI_LIB_SOURCES
    AppConfig.cpp
    Common.cpp
    CloudClient.cpp
    ConfigFileManager.cpp
    ConfigMessageConvertor.cpp
    CoreInterface.cpp
    DebugLevel.cpp
    DeviceManager.cpp
    DirectoryManager.cpp
    Hostname.cpp
    IpManager.cpp
    LogManager.cpp
    Multicast.cpp
    MulticastManager.cpp
    MulticastMessage.cpp
    ProcessCommand.cpp
    ProcessManager.cpp
    ScreenBBArrangement.cpp
    Screen.cpp
    ScreenListModel.cpp
    ScreenListSnapshotManager.cpp
    ScreenManager.cpp
    ServerInfo.cpp
)

add_library (synlib ${GUI_LIB_SOURCES})
target_compile_definitions (synlib PRIVATE -DSYNLIB)
qt5_use_modules (synlib Core Widgets Quick Network)
