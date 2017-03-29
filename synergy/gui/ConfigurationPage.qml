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

    ProcessManager {
        id: processManager
    }

    ConnectivityTester {
        id: connectivityTester
        cloudClient: applicationWindow.cloudClient
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

        Component.onCompleted: {
            applicationWindow.cloudClient.addScreen(localHostname.hostname())
        }

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

        // server configuration
        ScrollView {
            id: screenArrangementScrollView
            anchors.top: configurationPageBackgroundSeparator.bottom
            anchors.bottom: parent.bottom
            width: parent.width
            z: 1

            states: [
                State { when: !screenArrangementScrollView.visible;
                    PropertyChanges {
                        target: screenArrangementScrollView
                        opacity: 0.0
                    }
                }
            ]

            transitions: Transition {
                NumberAnimation { property: "opacity"; duration: 700}
            }

            Item {
                id: screenArrangement
                anchors.fill: parent
                scale: screenListModel.scale

                Repeater {
                    model: screenListModel
                    Item {
                        id: screenIcon
                        x: posX; y: posY
                        width: screenListModel.screenIconWidth()
                        height: screenListModel.screenIconHeight()
                        property point beginDrag
                        property int modelIndex

                        MouseArea {
                            id: mouse
                            anchors.fill: parent
                            drag.target: screenIcon
                            drag.axis: Drag.XandYAxis
                            hoverEnabled: true
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
                            }
                        }

                        Image {
                            id: itemImage
                            parent: screenIcon
                            anchors.fill: parent
                            fillMode: Image.Stretch
                            smooth: true
                            source: stateImage

                            Text {
                                width: parent.width - 20
                                anchors.verticalCenter: parent.verticalCenter
                                anchors.horizontalCenter: parent.horizontalCenter
                                text: name
                                font.pixelSize: 15
                                minimumPixelSize: 15
                                color: "white"
                                font.family: "Tahoma"
                                fontSizeMode: Text.HorizontalFit
                                horizontalAlignment: Text.AlignHCenter
                                elide: Text.ElideRight
                            }
                        }
                    }
                }
            }
        }
    }
}
