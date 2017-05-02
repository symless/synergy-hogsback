import QtQuick 2.0
import QtQuick.Controls 1.2

import com.synergy.gui 1.0

Rectangle {

    Item {
        id: profilePage
        anchors.fill: parent

        Rectangle {
            id: rectangle1
            anchors.fill: parent
            color:"#3f95b8"

            Version {
                anchors.left: parent.left
                anchors.leftMargin: 5
                anchors.bottom: parent.bottom
                anchors.bottomMargin: 5
            }

            // background header
            Rectangle {
                id: activationPageBackgroundHeader
                anchors.top: parent.top
                width: parent.width
                height: 94
                color:"white"

                Image {
                    id: logo
                    fillMode :Image.PreserveAspectFit
                    anchors.bottom: parent.bottom
                    anchors.bottomMargin: 15
                    anchors.top: parent.top
                    anchors.topMargin: 15
                    anchors.horizontalCenter: parent.horizontalCenter
                    source: "res/image/synergy-logo.png"
                }
            }

            // separator
            Rectangle {
                id: activationPageBackgroundSeparator
                anchors.top: activationPageBackgroundHeader.bottom
                width: parent.width
                height: 7
                color:"#96C13D"
            }

            Rectangle {
                id: profileArea
                width: 350
                height: 235
                color: "black"
                anchors.horizontalCenter: parent.horizontalCenter
                anchors.verticalCenter: parent.verticalCenter

                Text {
                    id: hint
                    z: 1
                    color: "White"
                    font.family: "Tahoma"
                    font.bold: false
                    horizontalAlignment: Text.AlignHCenter
                    anchors.top: signInArea.bottom
                    anchors.topMargin: 10
                    anchors.horizontalCenter: signInArea.horizontalCenter
                    font.pixelSize: 10
                    text: "This is the profile page"
                }
            }
        }
    }
}
