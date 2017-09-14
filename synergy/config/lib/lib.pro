TEMPLATE = lib
TARGET = guilib
QT += qml quick widgets network
DEFINES += SYNLIB

debug {
    DESTDIR = ../../bin/debug
}
release {
    DESTDIR = ../../bin/release
}

HEADERS += \
    src/Hostname.h \
    src/MulticastManager.h \
    src/MulticastMessageType.h \
    src/MulticastMessage.h \
    src/ServerInfo.h \
    src/Ipc.h \
    src/IpcClient.h \
    src/IpcReader.h \
    src/LogManager.h \
    src/ProcessManager.h \
    src/ProcessCommand.h \
    src/DebugLevel.h \
    src/DeviceManager.h \
    src/Screen.h \
    src/ScreenModel.h \
    src/ScreenManager.h \
    src/IScreenArrangement.h \
    src/ScreenBBArrangement.h \
    src/AppConfig.h \
    src/ConfigFileManager.h \
    src/Direction.h \
    src/LibMacro.h \
    src/Common.h \
    src/DirectoryManager.h \
    src/CoreInterface.h \
    src/ProcessMode.h \
    src/ConfigMessageConvertor.h \
    src/Multicast.h \
    src/IpManager.h \
    src/ScreenState.h

SOURCES += \
    src/Hostname.cpp \
    src/MulticastManager.cpp \
    src/MulticastMessage.cpp \
    src/ServerInfo.cpp \
    src/Ipc.cpp \
    src/IpcClient.cpp \
    src/IpcReader.cpp \
    src/LogManager.cpp \
    src/ProcessManager.cpp \
    src/ProcessCommand.cpp \
    src/DebugLevel.cpp \
    src/DeviceManager.cpp \
    src/Screen.cpp \
    src/ScreenModel.cpp \
    src/ScreenManager.cpp \
    src/ScreenBBArrangement.cpp \
    src/AppConfig.cpp \
    src/ConfigFileManager.cpp \
    src/Common.cpp \
    src/DirectoryManager.cpp \
    src/CoreInterface.cpp \
    src/ConfigMessageConvertor.cpp \
    src/Multicast.cpp \
    src/IpManager.cpp
