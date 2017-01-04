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

//  IpcClient {
//      id: ipcClient
//  }

    ProcessManager {
        id: processManager
        //ipcClient: ipcClient;
    }

    ScreenManager {
        id: screenManager
        screenListModel: screenListModel
        processManager: processManager
        viewWidth: screenArrangementScrollView.width
        viewHeight: screenArrangementScrollView.height
    }

    // TODO: This is for testing, remove before release
    Connections {
        target: applicationWindow
        onKeyReceived: {
            screenManager.saveSnapshot()
        }
    }

    Item {
        id: configurationPage
        anchors.top: parent.top
        anchors.bottom: parent.bottom
        anchors.left: navigationMenu.right
        width: parent.width

        // background header
        Rectangle {
            id: configurationPageBackgroundHeader
            anchors.top: parent.top
            width: parent.width
            height: 70
            color:"white"

            // navigation icon
            Rectangle {
                width: 30
                height: 30
                z: 1
                anchors.left: parent.left
                anchors.leftMargin: 20
                anchors.bottom: parent.bottom
                anchors.bottomMargin: 20
                color: "transparent"

                MouseArea {
                    id: navigationMouseArea
                    anchors.fill: parent
                    hoverEnabled: true
                    onPressed: {
                        if (navigationMenu.opened) {
                            navigationMenu.x = -navigationMenu.width
                        }
                        else {
                            navigationMenu.x = 0
                        }

                        navigationMenu.opened = !navigationMenu.opened
                    }
                }
                Image {
                    id: navigationImage
                    width: parent.width
                    height: parent.height
                    fillMode: Image.Stretch
                    smooth: true
                    source: "qrc:/res/image/navigation.png"
                }
            }

            // option icon
            Rectangle {
                id: option
                width: 30
                height: 30
                z: 1
                anchors.right: parent.right
                anchors.rightMargin: 20
                anchors.bottom: parent.bottom
                anchors.bottomMargin: 20
                color: "transparent"

                MouseArea {
                    id: optionMouseArea
                    anchors.fill: parent
                    hoverEnabled: true
                    onPressed: {
                        dropMenu.state = dropMenu.state === "dropDown" ?
                                    "" : "dropDown"
                    }
                }

                Image {
                    id: optionImage
                    width: parent.width
                    height: parent.height
                    fillMode: Image.Stretch
                    smooth: true
                    source: "qrc:/res/image/option.png"
                }
            }

            // search icon
            Rectangle {
                id: search
                width: 50
                height: 50
                z: 1
                anchors.right: option.left
                anchors.rightMargin: 10
                anchors.bottom: parent.bottom
                anchors.bottomMargin: 10
                color: "transparent"
                property bool searching: true

                MouseArea {
                    id: searchMouseArea
                    anchors.fill: parent
                    hoverEnabled: true
                    onPressed: {
                        if (search.searching) {
                            search.height = 38
                            searchImage.source =
                                    "qrc:/res/image/signal-tower-off.png"
                        }
                        else {
                             search.height = 50
                            searchImage.source =
                                    "qrc:/res/image/signal-tower-on.png"
                        }

                        search.searching = !search.searching
                    }
                }

                Image {
                    id: searchImage
                    width: parent.width
                    height: parent.height
                    fillMode: Image.PreserveAspectFit
                    smooth: true
                    source: "qrc:/res/image/signal-tower-on.png"
                }
            }
        }

        // separator
        Rectangle {
            id: configurationPageBackgroundSeparator
            anchors.top: configurationPageBackgroundHeader.bottom
            width: parent.width
            height: 7
            color:"#96C13D"
        }

        // drop menu
        Rectangle {
            id: dropMenu
            width:120;
            z: 1
            anchors.right: parent.right
            anchors.top: configurationPageBackgroundSeparator.bottom
            property variant items: ["Undo", "Redo", "Add a new screen"]
            smooth:true;

            Rectangle {
                id:dropDown
                width: parent.width;
                height:0;
                clip:true;
                radius:4;
                anchors.top: parent.top;
                anchors.margins: 2;
                color: "lightgray"

                ListView {
                    id:listView
                    height:200;
                    model: dropMenu.items
                    currentIndex: 0
                    delegate: Item{
                        width:dropMenu.width;
                        height: 30;

                        Rectangle {
                            anchors.fill: parent
                            color: dropDownItemMouseArea.containsMouse ? "white" : "transparent"
                        }

                        Text {
                            text: modelData
                            anchors.horizontalCenter: parent.horizontalCenter;
                            anchors.verticalCenter: parent.verticalCenter;
                        }

                        MouseArea {
                            id: dropDownItemMouseArea
                            anchors.fill: parent;
                            hoverEnabled: true
                            onClicked: {
                                dropMenu.state = ""
                            }
                        }
                    }
                }
            }

            states: State {
                name: "dropDown";
                PropertyChanges { target: dropDown; height: 30 * dropMenu.items.length }
            }

            transitions: Transition {
                NumberAnimation { target: dropDown; properties: "height";
                    easing.type: Easing.OutExpo; duration: 1000 }
            }
        }

        // background
        Rectangle {
            id: configurationPageBackground
            anchors.top: configurationPageBackgroundSeparator.bottom
            width: parent.width
            anchors.bottom: parent.bottom
            color:"#3F95B8"
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
                            }
                            onReleased: {
                                screenManager.moveModel(modelIndex,
                                                screenIcon.x - beginDrag.x,
                                                screenIcon.y - beginDrag.y)
                            }
                            onHoveredChanged: {
                                shutdownImage.visible = !shutdownImage.visible
                            }
                        }

                        Image {
                            id: itemImage
                            parent: screenIcon
                            anchors.fill: parent
                            fillMode: Image.Stretch
                            smooth: true
                            source: stateImage

                            Image {
                                id: shutdownImage
                                width: 20
                                height: 20
                                anchors.top: parent.top
                                anchors.topMargin: 9
                                anchors.right: parent.right
                                anchors.rightMargin: 9
                                fillMode: Image.PreserveAspectFit
                                smooth: true
                                visible: false
                                source: "qrc:/res/image/shutdown.png"
                            }

                            Image {
                                id: signalImage
                                width: 20
                                height: 20
                                anchors.top: parent.top
                                anchors.topMargin: 9
                                anchors.left: parent.left
                                anchors.leftMargin: 9
                                fillMode: Image.PreserveAspectFit
                                smooth: true
                                visible: stateImage === "qrc:/res/image/screen-active.png"
                                source: "qrc:/res/image/signal.png"
                            }

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

    // navigation menu
    Rectangle {
        id: navigationMenu
        height: parent.height
        width: 150
        x: -width
        z: 2
        color:"white"
        property bool opened: false

        Behavior on x {NumberAnimation
                        {duration: 500; easing.type: Easing.OutQuad}}

        // profile icon
        Rectangle {
            id: profile
            width: parent.width
            height: width
            z: 3
            anchors.horizontalCenter: parent.horizontalCenter
            anchors.top: parent.top
            anchors.topMargin: 20
            color: "transparent"

            Image {
                id: profileImage
                width: parent.width
                height: parent.height
                fillMode: Image.Stretch
                smooth: true
                source: "qrc:/res/image/pro_user.png"
            }
        }

        // configure icon
        Rectangle {
            id: configure
            width: 50
            height: 50
            z: 3
            anchors.horizontalCenter: parent.horizontalCenter
            anchors.top: profile.bottom
            anchors.topMargin: 30
            color: "transparent"

            Image {
                id: configureImage
                width: parent.width
                height: parent.height
                fillMode: Image.Stretch
                smooth: true
                source: "qrc:/res/image/configuration.png"
            }
        }

        // general icon
        Rectangle {
            id: general
            width: 50
            height: 50
            z: 3
            anchors.horizontalCenter: parent.horizontalCenter
            anchors.top: configure.bottom
            anchors.topMargin: 30
            color: "transparent"

            Image {
                id: generalImage
                width: parent.width
                height: parent.height
                fillMode: Image.Stretch
                smooth: true
                source: "qrc:/res/image/general.png"
            }
        }

        // info icon
        Rectangle {
            id: info
            width: 50
            height: 50
            z: 3
            anchors.horizontalCenter: parent.horizontalCenter
            anchors.top: general.bottom
            anchors.topMargin: 30
            color: "transparent"

            Image {
                id: infoImage
                width: parent.width
                height: parent.height
                fillMode: Image.Stretch
                smooth: true
                source: "qrc:/res/image/info.png"
            }
        }
    }
}
