import QtQuick 2.5
import QtQuick.Controls 1.4
import com.synergy.gui 1.0

Rectangle {
    Item {
        id: accessibilityPage
        anchors.fill: parent

        Timer {
            id: accessibilityPullingTimer
            interval: 500; running: true; repeat: true
            onTriggered: {
                if (accessibilityManager.processHasAccessibility()) {
                     accessibilityPullingTimer.stop()
                     stackView.push (stackView.nextPage())
                 }
            }
        }

        Rectangle {
            id: rectangle1
            anchors.fill: parent
            color:"#3F95B8"

            Version {
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
                anchors.top: accessibilityPageBackgroundSeparator.bottom
                anchors.topMargin: 30
                anchors.bottom: rectangle1.bottom
                anchors.bottomMargin: 130

                AnimatedImage {
                    id: tutorial
                    anchors.fill: parent
                    fillMode: Image.PreserveAspectFit
                    source: "qrc:/res/image/accessibility-tutorial.gif"

                    onFrameChanged: {
                        if (tutorial.currentFrame <= 105) {
                            hint.text = "Click the padlock at the bottom left"
                        }
                        else if (tutorial.currentFrame <= 209) {
                            hint.text = "You will be asked to enter your macOS password"
                        }
                        else if (tutorial.currentFrame <= 273) {
                            hint.text = "Tick Synergy in the app list"
                        }
                        else {
                            hint.text = "Close Security & Privacy Preferences"
                        }
                    }
                }

                HeaderText {
                    id: hint
                    color: "white"
                    anchors.horizontalCenter: parent.horizontalCenter
                    anchors.top: tutorial.bottom
                    anchors.topMargin: 10
                    horizontalAlignment: Text.AlignHCenter

                    Behavior on text {
                        SequentialAnimation {
                            NumberAnimation { target: hint; property: "opacity"; to: 0 }
                            PropertyAction {}
                            NumberAnimation { target: hint; property: "opacity"; to: 1 }
                        }
                    }
                }

                SynergyButton {
                    id: openAccessibilityButton
                    anchors.horizontalCenter: parent.horizontalCenter
                    anchors.top: hint.bottom
                    anchors.topMargin: 25
                    buttonText: "Open Security & Privacy Preferences"

                    onButtonClicked: {
                        accessibilityManager.openAccessibilityDialog()
                    }
                }
            }
        }
    }
}
