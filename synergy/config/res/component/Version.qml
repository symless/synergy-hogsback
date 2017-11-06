import QtQuick 2.5
import QtQuick.Controls 1.4

import com.synergy.gui 1.0

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
        anchors.bottom: parent.bottom
        color: textColor
        text: "Version: Synergy " + VersionManager.buildVersion()
    }

    BodyText {
        id: versionWarning
        anchors.left: parent.left
        anchors.bottom: versionNumber.top
        color: textColor
        text: VersionManager.latestVersion().length === 0 ?
                "" : VersionManager.latestVersion() + " is available! Please <a href='https://symless.com/synergy/downloads'>download</a>."
        onLinkActivated: Qt.openUrlExternally(link)

        MouseArea {
            anchors.fill: parent
            acceptedButtons: Qt.NoButton
            cursorShape: parent.hoveredLink ? Qt.PointingHandCursor : Qt.ArrowCursor
        }
    }

    Connections {
        target: VersionManager
        onNewVersionDetected: {
            versionWarning.text = newVersion + " is available! Please <a href='https://symless.com/synergy/downloads'>download</a>"
        }
    }
}
