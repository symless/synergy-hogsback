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
                id: curtain
                anchors.top: profilePageBackgroundSeparator.bottom
                anchors.bottom: parent.bottom
                width: parent.width
                color: "black"

                HeaderText {
                    id: hint
                    color: "white"
                    horizontalAlignment: Text.AlignHCenter
                    anchors.top: curtain.top
                    anchors.topMargin: (curtain.height - profileArea.height) / 4 - dp(7)
                    anchors.horizontalCenter: profileArea.horizontalCenter
                    text: "Synergy doesn't know where you are\nPlease click on a profile below to join or create a new profile"
                }

                Rectangle {
                    id: profileArea
                    width: parent.width * 0.75
                    height: parent.height * 0.618
                    anchors.horizontalCenter: parent.horizontalCenter
                    anchors.verticalCenter: parent.verticalCenter
                    color: "white"

                    Rectangle {
                        id: profileList
                        width: dp(120)
                        height: profileArea.height - newProfileButton.height
                        anchors.left: profileArea.left
                        anchors.top: profileArea.top
                        color: "green"
                    }

                    Rectangle {
                        id: newProfileButton
                        width: profileList.width
                        height: dp(25)
                        anchors.left: profileArea.left
                        anchors.bottom: parent.bottom
                        color: "blue"
                    }

                    Rectangle {
                        id: preview
                        anchors.left: profileList.right
                        anchors.right: profileArea.right
                        anchors.top: profileArea.top
                        anchors.bottom: profileArea.bottom

                        color: "red"
                    }
                }
            }
        }
    }
}
