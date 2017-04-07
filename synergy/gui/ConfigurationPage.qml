import QtQuick 2.0
import QtQuick.Controls 1.2

import com.synergy.gui 1.0

Rectangle {

    Hostname {
        id: localHostname
    }

    ScreenListModel {
        id: screenListModel
    }

    ConnectivityTester {
        id: connectivityTester
        cloudClient: applicationWindow.cloudClient
    }

    ProcessManager {
        id: processManager
        connectivityTester: connectivityTester
    }

    ScreenManager {
        id: screenManager
        screenListModel: screenListModel
        processManager: processManager
        viewWidth: screenArrangementScrollView.width
        viewHeight: screenArrangementScrollView.height
        cloudClient: applicationWindow.cloudClient
    }

    // TODO: This is for testing, remove before release
    Connections {
        target: applicationWindow
        onKeyReceived: {
            screenManager.onKeyPressed(key)

            if (key == Qt.Key_QuoteLeft) {
                if (logConsole.height === 0) {
                    logConsole.height = 100
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
            applicationWindow.cloudClient.addScreen(localHostname.hostname())
        }

        // version label
        Version {
            anchors.left: parent.left
            anchors.leftMargin: 5
            anchors.bottom: parent.bottom
            anchors.bottomMargin: 5
        }

        // background header
        Rectangle {
            id: configurationPageBackgroundHeader
            anchors.top: parent.top
            width: parent.width
            height: 70
            color:"white"
            z: 1

            // Synergy logo
            Image {
                id: logoImage
                anchors.left: parent.left
                anchors.leftMargin: 15
                anchors.verticalCenter: parent.verticalCenter
                height: parent.height - 20
                fillMode: Image.PreserveAspectFit
                smooth: true
                source: "qrc:/res/image/synergy-icon.png"
            }
        }

        // separator
        Rectangle {
            id: configurationPageBackgroundSeparator
            anchors.top: configurationPageBackgroundHeader.bottom
            width: parent.width
            height: 7
            color:"#96C13D"
            z: 1
        }

        // separator 2, which is coinciding with the saparator above in the beginning
        // when console expands out, this is at the bottom of the console
        Rectangle {
            id: configurationPageBackgroundSeparator2
            anchors.top: logConsole.bottom
            width: parent.width
            height: logConsole.height > 0 ? 7 : 0
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

            ListView {
                id: logConsoleListView
                anchors.fill: parent

                model: LoggingModel
                delegate: Text {
                    width: parent.width
                    text: modelData
                    color: "white"
                    wrapMode: Text.WordWrap
                }

                // always focus on the last line
                onCountChanged: {
                    logConsoleListView.positionViewAtEnd()
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

        // hostname
        Text {
            id: hostname
            z: 1
            text: localHostname.hostname()
            font.family: "Tahoma"
            font.bold: false
            verticalAlignment: Text.AlignVCenter
            horizontalAlignment: Text.AlignHCenter
            anchors.top: parent.top
            anchors.topMargin: 15
            anchors.horizontalCenter: parent.horizontalCenter
            font.pixelSize: 30
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
                        x: posX; y: posY
                        width: screenListModel.screenIconWidth()
                        height: screenListModel.screenIconHeight()
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
                                                screenIcon.x + 45,
                                                screenIcon.y + 45)
                                screenManager.lockScreen(modelIndex)
                            }
                            onReleased: {
                                screenManager.moveModel(modelIndex,
                                                screenIcon.x - beginDrag.x,
                                                screenIcon.y - beginDrag.y)
                                screenManager.unlockScreen(modelIndex)
                                modelIndex = -1
                            }

                            onClicked: {
                                // right click activate and deactivate edit mode
                                if(mouse.button === Qt.RightButton) {
                                    screenIcon.editMode = !screenIcon.editMode;
                                    if (screenIcon.editMode === false) {
                                        screenImage.source = stateImage
                                    }
                                    else {
                                        screenImage.source = "qrc:/res/image/screen-edit.png"
                                    }
                                }
                            }
                        }

                        // selected border
                        Rectangle {
                            anchors.fill: parent
                            color: "transparent"
                            border.color: "white"
                            border.width: 2
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
                            source: stateImage

                            // screen name
                            Text {
                                id: screenNameText
                                width: parent.width - 20
                                anchors.verticalCenter: parent.verticalCenter
                                anchors.horizontalCenter: parent.horizontalCenter
                                text: name
                                font.pixelSize: 15
                                minimumPixelSize: 15
                                color: screenState == "Connected" ? "black" : "white"
                                font.family: "Tahoma"
                                fontSizeMode: Text.HorizontalFit
                                horizontalAlignment: Text.AlignHCenter
                                elide: Text.ElideRight
                                visible: screenImage.source != "qrc:/res/image/screen-edit.png"
                            }

                            // unsubscrible button in edit mode
                            Image {
                                id: screenUnsubIcon
                                parent: screenImage
                                width: 35
                                height: width
                                anchors.right: parent.horizontalCenter
                                anchors.rightMargin: 5
                                anchors.verticalCenter: parent.verticalCenter
                                fillMode: Image.PreserveAspectFit
                                smooth: true
                                visible: screenImage.source == "qrc:/res/image/screen-edit.png"
                                source: "qrc:/res/image/unsub.png"
                            }

                            // furthur edit button in edit mode
                            Image {
                                id: screenEditIcon
                                parent: screenImage
                                width: 35
                                height: width
                                anchors.left: parent.horizontalCenter
                                anchors.leftMargin: 5
                                anchors.verticalCenter: parent.verticalCenter
                                fillMode: Image.PreserveAspectFit
                                smooth: true
                                visible: screenImage.source == "qrc:/res/image/screen-edit.png"
                                source: "qrc:/res/image/edit.png"
                            }

                            // connecting prograss bar background
                            Rectangle {
                                id: connectingBar
                                visible: screenState == "Connecting" && screenImage.source != "qrc:/res/image/screen-edit.png"
                                anchors.horizontalCenter: parent.horizontalCenter
                                anchors.bottom: parent.bottom
                                anchors.bottomMargin: 15
                                width: parent.width / 7 * 5
                                height: 5
                                clip: true
                                z: 1
                                color: "#828282"

                                // connecting prograss sliding bar
                                Rectangle {
                                    id: connectingSlidingBar
                                    x: -width
                                    width: 35
                                    height: 5
                                    color: "#96C13D"

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
                        }
                    }
                }
            }
        }
    }
}
