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
                color: "white"

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
                    border.width: 1

                    Rectangle {
                        id: profileList
                        width: dp(120)
                        height: profileArea.height - newProfileButton.height + 1
                        anchors.left: profileArea.left
                        anchors.top: profileArea.top
                        border.width: 1
                    }

                    Rectangle {
                        id: newProfileButton
                        width: profileList.width
                        height: dp(25)
                        anchors.left: profileArea.left
                        anchors.bottom: parent.bottom
                        border.width: 1

                        Image {
                            id: addNewProfile
                            parent: newProfileButton
                            width: dp(25)
                            height: width
                            anchors.left: parent.left
                            anchors.leftMargin: dp(4)
                            anchors.verticalCenter: parent.verticalCenter
                            fillMode: Image.PreserveAspectFit
                            smooth: true
                            source: "qrc:/res/image/unsub.png"

                            MouseArea {
                                id: addNewProfileMouseArea
                                anchors.fill: parent
                                onPressed: {

                                }
                            }
                        }

                        HeaderText {
                            id: addNewProfileText
                            anchors.left: addNewProfile.right
                            anchors.verticalCenter: parent.verticalCenter
                            anchors.leftMargin: dp(4)
                            text: "New Profile"
                        }
                    }

                    Rectangle {
                        id: previewBorder
                        anchors.left: profileList.right
                        anchors.right: profileArea.right
                        anchors.top: profileArea.top
                        anchors.bottom: profileArea.bottom
                        border.width: 1

                        Rectangle {
                            id: preview
                            anchors.fill: parent
                            anchors.topMargin: 1
                            anchors.rightMargin: 1

                            Rectangle {
                                id: profilePreviewBackgroundHeader
                                anchors.top: parent.top
                                width: parent.width
                                height: dp(36)
                                color: "white"
                            }

                            Rectangle {
                                id: profilePreviewBackgroundSeparator
                                anchors.top: profilePreviewBackgroundHeader.bottom
                                width: parent.width
                                height: dp(3)
                                color:"#96C13D"
                            }

                            Rectangle {
                                id: profilePreviewBackground
                                anchors.top: profilePreviewBackgroundSeparator.bottom
                                width: parent.width
                                anchors.bottom: parent.bottom
                                color:"#3F95B8"
                            }
                        }
                    }
                }
            }
        }
    }
}
