import QtQuick 2.0
import QtWebEngine 1.0
import QtQuick.Controls 1.2
import QtQuick.Window 2.0

Window {
    width: 500
    height: 350
    WebEngineView {
        anchors.fill: parent
        url: "http://www.qt.io"
    }
}
