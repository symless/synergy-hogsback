import QtQuick 2.3
import QtQuick.Controls 1.2

Rectangle {
    width: 190
    height: 30
    color: "#00000000"
    z: 1

    property color textColor :  "white"
    property string fontFamily : "Sans"

    Text {
        id: versionNumber
        anchors.left: parent.left
        color: textColor
        text: "Version: Synergy 2.0.0-alpha2"
        font.family: fontFamily
        font.pointSize: 11
        renderType: Text.NativeRendering
    }

    Text {
        anchors.left: parent.left
        anchors.top: versionNumber.bottom
        color: textColor
        text: "Alpha testing only, do not redistribute"
        font.family: fontFamily
        font.pointSize: 11
        renderType: Text.NativeRendering
    }
}
