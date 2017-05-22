import QtQuick 2.5
import QtQuick.Controls 1.4

Rectangle {
    width: 190
    height: 30
    anchors.left: parent.left
    anchors.leftMargin: 5
    anchors.bottom: parent.bottom
    anchors.bottomMargin: 5
    color: "#00000000"
    z: 1

    property color textColor :  "white"

    BodyText {
        id: versionNumber
        anchors.left: parent.left
        anchors.bottom: versionWarning.top
        color: textColor
        text: "Version: Synergy 2.0-alpha3"
    }

    BodyText {
        id: versionWarning
        anchors.left: parent.left
        anchors.bottom: parent.bottom
        color: textColor
        text: "Alpha testing only, do not redistribute"
    }
}
