TEMPLATE = app
QT += qml quick widgets network
SOURCES += main.cpp

debug {
    DESTDIR = ../../bin/debug
    LIBS += -L../../bin/debug -lsynlib
}
release {
    DESTDIR = ../../bin/release
    LIBS += -L../../bin/release -lsynlib
}

# Default rules for deployment.
include(deployment.pri)

RESOURCES += qml.qrc

INCLUDEPATH += ../lib/src

RC_ICONS = res/image/logo.ico
