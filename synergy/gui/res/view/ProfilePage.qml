import QtQuick 2.5
import QtQuick.Controls 1.4

import com.synergy.gui 1.0

Rectangle {
    Item {
        id: profilePage
        anchors.fill: parent

        Rectangle {
            id: rectangle1
            anchors.fill: parent
            color:"#E5E4E4"
            border.width: 1
            border.color: "black"

            Version {
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
                width: parent.width * 0.618
                height: parent.height * 0.618
                color: "black"
                anchors.horizontalCenter: parent.horizontalCenter
                anchors.verticalCenter: parent.verticalCenter

                Text {
                    id: hint
                    z: 1
                    color: "black"
                    font.family: "Tahoma"
                    font.bold: false
                    horizontalAlignment: Text.AlignHCenter
                    anchors.top: profileArea.bottom
                    anchors.topMargin: 10
                    anchors.horizontalCenter: profileArea.horizontalCenter
                    font.pixelSize: 10
                    text: "This is the profile page"
                }
            }
        }
    }
}
