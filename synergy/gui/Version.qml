import QtQuick 2.3
import QtQuick.Controls 1.2

Rectangle {
    width: 190
    height: 30
    color: "#00000000"
    z: 1

    Text {
        id: versionNumber
        anchors.left: parent.left
        color: "white"
        text: "Version: Synergy 2.0.0-alpha2"
    }

    Text {
        anchors.left: parent.left
        anchors.top: versionNumber.bottom
        color: "white"
        text: "Alpha testing only, do not redistribute"
    }
}
