import QtQuick 2.5
import QtQuick.Controls 1.4
import QtQuick.Controls.Styles 1.4

import com.synergy.gui 1.0

Rectangle {

    Hostname {
        id: localHostname
    }

    ScreenListModel {
        id: screenListModel
    }

    ScreenManager {
        id: screenManager
        screenListModel: screenListModel
        processManager: applicationWindow.processManager
        //viewWidth: screenArrangementScrollView.width
        //viewHeight: screenArrangementScrollView.height
    }

    Connections {
        target: applicationWindow
        onKeyReceived: {
            screenManager.onKeyPressed(key)

            if (key == Qt.Key_QuoteLeft) {
                if (logConsole.height === 0) {
                    logConsole.height = dp(70)
                }
                else {
                    logConsole.height = 0
                }
            }
        }
    }

    Item {
        id: configurationPage
        anchors.top: parent.top
        anchors.bottom: parent.bottom
        width: parent.width

        // add localhost as the initial screen
        Component.onCompleted: {
            CloudClient.switchProfile()
            CloudClient.userProfiles()
        }

        // version label
        Version {
            z: 2
        }

        // background header
        Rectangle {
            id: configurationPageBackgroundHeader
            anchors.top: parent.top
            width: parent.width
            height: dp(65)
            color: "white"
            z: 2

            // hostname
            LogoText {
                id: hostname
                z: 2
                text: localHostname.hostname()
                color: "#4D4D4D"
                verticalAlignment: Text.AlignVCenter
                horizontalAlignment: Text.AlignHCenter
                anchors.verticalCenter: parent.verticalCenter
                anchors.horizontalCenter: parent.horizontalCenter
                //fillMode: Image.PreserveAspectFit
                smooth: true
                //source: "qrc:/res/image/synergy-icon.png"
            }
        }

        // separator
        Rectangle {
            id: configurationPageBackgroundSeparator
            anchors.top: configurationPageBackgroundHeader.bottom
            width: parent.width
            height: dp(5)
            color:"#96C13D"
            z: 1
        }

        // separator 2, which is coinciding with the saparator above in the beginning
        // when console expands out, this is at the bottom of the console
        Rectangle {
            id: configurationPageBackgroundSeparator2
            anchors.top: logConsole.bottom
            width: parent.width
            height: logConsole.height > 0 ? dp(5) : 0
            color:"#96C13D"
            z: 3
        }

        // log console
        Rectangle {
            id: logConsole
            anchors.top: configurationPageBackgroundSeparator.bottom
            width: parent.width
            height: 0
            color:"black"
            clip: true
            z: 2
            Behavior on height {NumberAnimation {duration: 500; easing.type: Easing.OutQuad}}

            TextArea {
                id: logConsoleTextArea
                anchors.fill: parent
                readOnly: true
                wrapMode: Text.WordWrap
                text: LoggingModel

                BodyText {
                    id: sample
                }

                style: TextAreaStyle {
                    backgroundColor: "black"
                    textColor: "white"
                    font.family: sample.font.family
                    font.pixelSize: sample.font.pixelSize
                }

                menu: Menu {
                    MenuItem {
                        text: "Copy"
                        shortcut: "Ctrl+C"
                        onTriggered: {
                            logConsoleTextArea.selectAll()
                            logConsoleTextArea.copy()
                        }
                    }

                    MenuItem {
                        text: "&Send log"
                        shortcut: "Alt+S"
                        onTriggered: {
                            LogManager.uploadLogFile()
                        }
                    }

                    style: MenuStyle {
                        font.family: sample.font.family
                        font.pixelSize: sample.font.pixelSize
                    }
                }

                // always focus on the last line
                onLineCountChanged: {
                    logConsoleTextArea.__verticalScrollBar.value = logConsoleTextArea.__verticalScrollBar.maximumValue
                }
            }
        }

        // background
        Rectangle {
            id: configurationPageBackground
            anchors.top: configurationPageBackgroundSeparator.bottom
            width: parent.width
            anchors.bottom: parent.bottom
            color:"#3F95B8"
            z: 1
        }

        // configuration area
        ScrollView {
            id: screenArrangementScrollView
            anchors.top: configurationPageBackgroundSeparator.bottom
            anchors.bottom: parent.bottom
            width: parent.width
            z: 1

            Item {
                id: screenArrangement
                anchors.fill: parent
                scale: screenListModel.scale

                Repeater {
                    model: screenListModel

                    // individual screen
                    Item {
                        id: screenIcon
                        x: dp(posX); y: dp(posY)
                        width: dp(screenListModel.screenIconWidth())
                        height: dp(screenListModel.screenIconHeight())
                        property point beginDrag
                        property int modelIndex: -1
                        property var editMode: false

                        // mouse area that covers the whole individual screen
                        MouseArea {
                            id: screenMouseArea
                            anchors.fill: parent
                            drag.target: screenIcon
                            drag.axis: Drag.XandYAxis
                            hoverEnabled: true
                            acceptedButtons: Qt.LeftButton | Qt.RightButton

                            onPressed: {
                                beginDrag = Qt.point(screenIcon.x,
                                                screenIcon.y);
                                modelIndex = screenManager.getModelIndex(
                                                op(screenIcon.x + 45),
                                                op(screenIcon.y + 45))
                                screenManager.lockScreen(modelIndex)
                            }
                            onReleased: {
                                screenManager.moveModel(modelIndex,
                                                op(screenIcon.x - beginDrag.x),
                                                op(screenIcon.y - beginDrag.y))
                                screenManager.unlockScreen(modelIndex)
                                modelIndex = -1
                            }
                        }

                        // selected border
                        Rectangle {
                            anchors.fill: parent
                            color: "transparent"
                            border.color: "white"
                            border.width: dp(1)
                            radius: 4
                            z: 1
                            visible: modelIndex === index
                        }

                        // screen image
                        Image {
                            id: screenImage
                            parent: screenIcon
                            anchors.fill: parent
                            fillMode: Image.Stretch
                            smooth: true
                            source: statusImage

                            // screen name
                            HeaderText {
                                id: screenNameText
                                width: parent.width - dp(14)
                                anchors.horizontalCenter: parent.horizontalCenter
                                anchors.top: parent.top
                                anchors.topMargin: parent.height / 4
                                text: name
                                color: screenStatus == "Connected" ? "black" : "white"
                                horizontalAlignment: Text.AlignHCenter
                                elide: Text.ElideRight
                                visible: screenImage.source != "qrc:/res/image/screen-edit.png"
                            }

                            // unsubscrible button in edit mode
                            Image {
                                id: screenUnsubIcon
                                parent: screenImage
                                width: dp(25)
                                height: width
                                anchors.right: parent.horizontalCenter
                                anchors.rightMargin: dp(4)
                                anchors.verticalCenter: parent.verticalCenter
                                fillMode: Image.PreserveAspectFit
                                smooth: true
                                visible: screenImage.source == "qrc:/res/image/screen-edit.png"
                                source: "qrc:/res/image/unsub.png"

                                MouseArea {
                                    id: unsubScreenMouseArea
                                    anchors.fill: parent
                                    onPressed: {
                                        //screenManager.removeScreen(name, true)
                                    }
                                }
                            }

                            // furthur edit button in edit mode
                            Image {
                                id: screenEditIcon
                                parent: screenImage
                                width: dp(25)
                                height: width
                                anchors.left: parent.horizontalCenter
                                anchors.leftMargin: dp(4)
                                anchors.verticalCenter: parent.verticalCenter
                                fillMode: Image.PreserveAspectFit
                                smooth: true
                                visible: screenImage.source == "qrc:/res/image/screen-edit.png"
                                source: "qrc:/res/image/edit.png"
                            }

                            // connecting prograss bar background
                            Rectangle {
                                id: connectingBar
                                visible: (screenStatus == "Connecting" || screenStatus == "ConnectingWithError") && screenImage.source != "qrc:/res/image/screen-edit.png"
                                anchors.horizontalCenter: parent.horizontalCenter
                                anchors.bottom: parent.bottom
                                anchors.bottomMargin: dp(15)
                                width: parent.width / 7 * 5
                                height: dp(4)
                                clip: true
                                z: 1
                                color: "#828282"

                                // connecting prograss sliding bar
                                Rectangle {
                                    id: connectingSlidingBar
                                    x: -width
                                    width: dp(25)
                                    height: dp(4)
                                    color: screenStatus == "Connecting" ? "#96C13D" : "red"
                                    z: 1
                                    states: [
                                        State {
                                            when: connectingBar.visible
                                            PropertyChanges {
                                                target: connectingSlidingBar
                                                x: connectingBar.width
                                            }
                                        }
                                    ]

                                    transitions: Transition {
                                        NumberAnimation { loops: Animation.Infinite; property: "x"; duration: 1500}
                                    }
                                }
                            }

                            // connecting error indication
                            Image {
                                id: errorIndication
                                sourceSize.width: dp(14)
                                sourceSize.height: sourceSize.width
                                anchors.horizontalCenter: connectingBar.horizontalCenter
                                anchors.verticalCenter: connectingBar.verticalCenter
                                visible: screenStatus == "ConnectingWithError"
                                smooth: false
                                source: "qrc:/res/image/error-indication.svg"
                                z: 2

                                MouseArea {
                                    anchors.fill: parent
                                    acceptedButtons: Qt.LeftButton

                                    onReleased: {
                                        errorMessageDialog.visible = !errorMessageDialog.visible
                                    }
                                }
                            }

                            Image {
                                id: errorMessageDialog
                                anchors.horizontalCenter: errorIndication.horizontalCenter
                                anchors.top: errorIndication.bottom
                                anchors.topMargin: dp(5)
                                width: dp(screenListModel.screenIconWidth() * 1.5)
                                height: dp(screenListModel.screenIconHeight() * 1.5)
                                smooth: true
                                fillMode: Image.PreserveAspectFit
                                source: "qrc:/res/image/error-message-dialog.png"
                                z: 2

                                Image {
                                    id: closeIcon
                                    anchors.top: errorMessageDialog.top
                                    anchors.topMargin: dp(15)
                                    anchors.right: errorMessageDialog.right
                                    anchors.rightMargin: dp(5)
                                    width: dp(10)
                                    height: width
                                    smooth: true
                                    source: "qrc:/res/image/close-icon.png"
                                    visible: errorMessageDialog.visible
                                    z: 2

                                    MouseArea {
                                        anchors.fill: parent
                                        acceptedButtons: Qt.LeftButton

                                        onReleased: {
                                            errorMessageDialog.visible = false
                                        }
                                    }
                                }

                                Text {
                                    id: errorMessageText
                                    width: parent.width
                                    font.pixelSize: dp(10)
                                    anchors.top: errorMessageDialog.top
                                    anchors.topMargin: dp(25)
                                    horizontalAlignment: Text.AlignHCenter
                                    wrapMode: Text.WordWrap
                                    text: errorMessage
                                }

                                Text {
                                    id: helpLinkText
                                    width: parent.width
                                    font.pixelSize: dp(10)
                                    anchors.top: errorMessageText.bottom
                                    anchors.topMargin: dp(15)
                                    horizontalAlignment: Text.AlignHCenter
                                    wrapMode: Text.WordWrap
                                    text: helpLink

                                    onLinkActivated: Qt.openUrlExternally(link)

                                    MouseArea {
                                        anchors.fill: parent
                                        acceptedButtons: Qt.NoButton
                                        cursorShape: parent.hoveredLink ? Qt.PointingHandCursor : Qt.ArrowCursor
                                    }
                                }
                            }
                        }
                    }
                }
            }
        }
    }
}
