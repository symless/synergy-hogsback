QT += testlib widgets
QT -= gui
TARGET = tests
CONFIG += console
CONFIG -= app_bundle
TEMPLATE = app

debug {
    DESTDIR = ../../bin/debug
    LIBS += -L../../bin/debug -lsynlib
}
release {
    DESTDIR = ../../bin/release
    LIBS += -L../../bin/release -lsynlib
}

# include the classes from the gui project
INCLUDEPATH += ../lib/src mock/

SOURCES += main.cpp \
    ConfigFileManagerTest.cpp \
    MulticastMessageTest.cpp \
    ProcessCommandTest.cpp \
    mock/MockDeviceManager.cpp \
    ScreenBBArrangementTest.cpp \
    ConfigMessageConvertorTest.cpp \
    mock/MockScreenModel.cpp \
    mock/MockDirectoryManager.cpp \
    ../lib/src/Common.cpp

HEADERS += AutoTest.h \
    ConfigFileManagerTest.h \
    MulticastMessageTest.h \
    ProcessCommandTest.h \
    mock/MockDeviceManager.h \
    ScreenBBArrangementTest.h \
    ConfigMessageConvertorTest.h \
    mock/MockScreenModel.h \
    mock/MockDirectoryManager.h \
    ../lib/src/Common.h
