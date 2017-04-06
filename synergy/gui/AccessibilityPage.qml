import QtQuick 2.0
import QtQuick.Controls 1.2
import QtQuick.Controls 2.1 as Controls
import QtGraphicalEffects 1.0

import com.synergy.gui 1.0

Rectangle {
    Item {
        id: accessibilityPage
        anchors.fill: parent

        Rectangle {
            id: rectangle1
            anchors.fill: parent
            color:"#e5e4e4"

            Version {
                anchors.left: parent.left
                anchors.leftMargin: 5
                anchors.bottom: parent.bottom
                anchors.bottomMargin: 5
                textColor: "black"
                fontFamily: "Myriad Pro"
            }

            // background header
            Rectangle {
                id: accessibilityPageBackgroundHeader
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
                id: accessibilityPageBackgroundSeparator
                anchors.top: accessibilityPageBackgroundHeader.bottom
                width: parent.width
                height: 7
                color:"#96C13D"
            }

            Rectangle {
                id: accessibilityArea
                width: 489
                color: "transparent"
                anchors.horizontalCenter: parent.horizontalCenter
                //anchors.verticalCenter: parent.verticalCenter
                anchors.top: accessibilityPageBackgroundSeparator.bottom
                anchors.topMargin: 30
                anchors.bottom: rectangle1.bottom
                anchors.bottomMargin: 30

                Text {
                    id: hint
                    color: "black"
                    font.bold: false
                    width: parent.width
                    horizontalAlignment: Text.AlignHCenter
                    font.pointSize: 14
                    font.family: "Myriad Pro"
                    text: "Synergy needs your help before it can control your Mac\n"
                          + "Follow the steps below to get going."
                    renderType: Text.NativeRendering
                }

                Rectangle {

                    id: accessibilityTutViewHolder
                    border.width: 10
                    border.color: "#333333"
                    radius: 5
                    antialiasing: true
                    width: parent.width
                    height: 275
                    anchors.top: hint.bottom
                    anchors.topMargin: 20
                    color:"#3f95b8"

                    Rectangle {
                        id: accessibilityTutViewHolderInner
                        anchors.left: parent.left
                        anchors.right: parent.right
                        anchors.leftMargin: 10
                        anchors.rightMargin: 10
                        height: 275
                        clip: true
                        color: "transparent"

                        Controls.SwipeView {
                            id: accessibilityTutView
                            anchors.fill: parent
                            currentIndex: 0

                            Rectangle {
                                color: "transparent"

                                Rectangle {
                                    anchors.left: parent.left
                                    anchors.right: parent.right
                                    anchors.margins: 15
                                    color: "transparent"
                                    anchors.verticalCenter: parent.verticalCenter
                                    height: 150

                                    Image {
                                        id: logo0
                                        source: "res/image/privacy-security.png"
                                        anchors.horizontalCenter: parent.horizontalCenter
                                        fillMode: Image.Pad
                                        visible: false
                                    }

                                    DropShadow {
                                        anchors.fill: logo0
                                        horizontalOffset: 5
                                        verticalOffset: 5
                                        radius: 8.0
                                        samples: 17
                                        color: "#30000000"
                                        source: logo0
                                    }

                                    MouseArea {
                                        anchors.fill: logo0
                                        hoverEnabled: true
                                        cursorShape: Qt.PointingHandCursor

                                        onClicked: {
                                           openAccessibilityButton.clicked()
                                        }
                                    }

                                    Text {
                                        id: gjhitghjet90
                                        color: "white"
                                        text: "Click the icon above to open Security & Privacy Preferences"
                                        font.family: "Myriad Pro"
                                        anchors.top: logo0.bottom
                                        anchors.topMargin: 15
                                        width: parent.width
                                        horizontalAlignment: Text.AlignHCenter
                                        wrapMode: Text.Wrap
                                        renderType: Text.NativeRendering
                                    }

                                    Button {
                                        id: openAccessibilityButton
                                        text: "Open Security && Privacy Preferences"
                                        anchors.horizontalCenter: parent.horizontalCenter
                                        anchors.top: gjhitghjet90.bottom
                                        anchors.topMargin: 15
                                        onClicked: {
                                            accessibilityManager.openAccessibilityDialog();
                                            accessibilityTutView.currentIndex += 1;
                                        }
                                        visible: false
                                    }
                                }
                            }

                            Rectangle {
                                color: "transparent"

                                Rectangle {
                                    anchors.left: parent.left
                                    anchors.right: parent.right
                                    anchors.margins: 15
                                    color: "transparent"
                                    height: 120
                                    clip: true
                                    anchors.verticalCenter: parent.verticalCenter

                                    Image {
                                        id: unlockSettingsImage
                                        fillMode: Image.Pad
                                        source: "res/image/unlock-settings.png"
                                        anchors.horizontalCenter: parent.horizontalCenter
                                        visible: false
                                    }

                                    DropShadow {
                                        anchors.fill: unlockSettingsImage
                                        horizontalOffset: 5
                                        verticalOffset: 5
                                        radius: 8.0
                                        samples: 17
                                        color: "#30000000"
                                        source: unlockSettingsImage
                                    }

                                    Text {
                                        id: gjhitghjet2
                                        color: "white"
                                        text: "In the dialog that just opened, click the padlock in the bottom left hand corner."
                                        font.family: "Myriad Pro"
                                        anchors.top: unlockSettingsImage.bottom
                                        anchors.topMargin: 15
                                        width: parent.width
                                        horizontalAlignment: Text.AlignHCenter
                                        wrapMode: Text.Wrap
                                        renderType: Text.NativeRendering
                                    }
                                }
                            }

                            Rectangle {
                                color: "transparent"

                                Rectangle {
                                    anchors.left: parent.left
                                    anchors.right: parent.right
                                    anchors.margins: 15
                                    color: "transparent"
                                    height: 210
                                    clip: true
                                    anchors.verticalCenter: parent.verticalCenter

                                    Image {
                                        id: typePasswordImage
                                        fillMode: Image.Pad
                                        source: "res/image/type-password.png"
                                        anchors.horizontalCenter: parent.horizontalCenter
                                    }

                                    DropShadow {
                                        anchors.fill: typePasswordImage
                                        horizontalOffset: 5
                                        verticalOffset: 5
                                        radius: 8.0
                                        samples: 17
                                        color: "#30000000"
                                        source: typePasswordImage
                                    }

                                    Text {
                                        id: gjhitghjet3
                                        color: "white"
                                        text: "You will be asked to enter your macOS password"
                                        font.family: "Myriad Pro"
                                        anchors.top: typePasswordImage.bottom
                                        anchors.margins: 15
                                        anchors.topMargin: 10
                                        width: parent.width
                                        horizontalAlignment: Text.AlignHCenter
                                        wrapMode: Text.Wrap
                                        renderType: Text.NativeRendering
                                    }
                                }
                            }

                            Rectangle {
                                color: "transparent"

                                Rectangle {
                                    anchors.left: parent.left
                                    anchors.right: parent.right
                                    anchors.margins: 15
                                    color: "transparent"
                                    height: 150
                                    clip: true
                                    anchors.verticalCenter: parent.verticalCenter

                                    Image {
                                        id: tickSynergyImage
                                        fillMode: Image.Pad
                                        source: "res/image/tick-synergy.png"
                                        anchors.horizontalCenter: parent.horizontalCenter
                                    }

                                    DropShadow {
                                        anchors.fill: tickSynergyImage
                                        horizontalOffset: 5
                                        verticalOffset: 5
                                        radius: 8.0
                                        samples: 17
                                        color: "#30000000"
                                        source: tickSynergyImage
                                    }

                                    Text {
                                        id: gjhitghjet4
                                        color: "white"
                                        text: "Tick Synergy in the app list"
                                        font.family: "Myriad Pro"
                                        anchors.top: tickSynergyImage.bottom
                                        anchors.margins: 15
                                        anchors.topMargin: 10
                                        width: parent.width
                                        horizontalAlignment: Text.AlignHCenter
                                        wrapMode: Text.Wrap
                                        renderType: Text.NativeRendering
                                    }
                                }
                            }
                        }

                    }

                    Controls.PageIndicator {
                        id: accessibilityTutPageIndicator
                        count: accessibilityTutView.count
                        currentIndex: accessibilityTutView.currentIndex
                        interactive: true

                        anchors.horizontalCenter: parent.horizontalCenter
                        anchors.bottom: accessibilityTutViewHolder.bottom
                        anchors.bottomMargin: 14
                        anchors.topMargin: 15

                        onCurrentIndexChanged: {
                            accessibilityTutView.currentIndex = currentIndex
                        }
                    }
                }

                Rectangle {
                    anchors.top: accessibilityTutViewHolder.bottom
                    anchors.topMargin: 15
                    height: 70
                    width: parent.width
                    color: "transparent"

                    Button {
                        id: checkAccessibilityButton
                        text: "I'm Done"
                        anchors.horizontalCenter: parent.horizontalCenter
                        anchors.top: parent.top
                        onClicked: {
                            if (accessibilityManager.processHasAccessibility()) {
                                stackView.push (stackView.nextPage())
                            } else {
                                checkAccessibilityFailureText.visible = true
                                accessibilityTutView.currentIndex = 0
                            }
                        }
                    }

                    Text {
                        id: checkAccessibilityFailureText
                        font.family: "Myriad Pro"
                        color: "black"
                        text: "Sorry, it looks like that didn't work.\n Follow the steps above and try again."
                        anchors.top: checkAccessibilityButton.bottom
                        anchors.topMargin: 10
                        wrapMode: Text.Wrap
                        width: parent.width
                        horizontalAlignment: Text.AlignHCenter
                        visible: false
                        renderType: Text.NativeRendering
                    }
                }

                Connections {
                    target: applicationWindow
                    onKeyReceived: {
                        if (key == Qt.Key_Return) {
                            if (accessibilityTutView.currentIndex == 0) {
                                openAccessibilityButton.clicked();
                                checkAccessibilityFailureText.visible = false
                            } else if ((accessibilityTutView.currentIndex == (accessibilityTutView.count - 1))) {
                                checkAccessibilityButton.clicked()
                            } else {
                                accessibilityTutView.currentIndex += 1
                                checkAccessibilityFailureText.visible = false
                            }
                        } else if (key == Qt.Key_Left) {
                                accessibilityTutView.currentIndex -= 1
                        } else if (key == Qt.Key_Right) {
                                accessibilityTutView.currentIndex += 1
                        }
                    }
                }
            }
        }
    }
}
