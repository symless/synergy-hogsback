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
                id: profilePageBackgroundHeader
                anchors.top: parent.top
                width: parent.width
                height: dp(65)
                color:"white"

                Image {
                    id: logo
                    fillMode :Image.PreserveAspectFit
                    anchors.bottom: parent.bottom
                    anchors.bottomMargin: dp(10)
                    anchors.top: parent.top
                    anchors.topMargin: dp(10)
                    anchors.horizontalCenter: parent.horizontalCenter
                    source: "res/image/synergy-logo.png"
                }
            }

            // separator
            Rectangle {
                id: profilePageBackgroundSeparator
                anchors.top: profilePageBackgroundHeader.bottom
                width: parent.width
                height: dp(5)
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
