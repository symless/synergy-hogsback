import QtQuick 2.5
import QtQuick.Controls 1.4
import QtQuick.Controls.Styles 1.4
import QtQuick.Dialogs 1.2

import com.synergy.gui 1.0

Rectangle {

    FontLoader {
        id: consoleFont;
        name: "Source Code Pro"
    }

    Hostname {
        id: localHostname
    }

    ScreenListModel {
        id: screenListModel
    }

    ScreenManager {
        id: screenManager
        screenListModel: screenListModel
        serviceProxy: applicationWindow.serviceProxy
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
        id: configPage
        anchors.top: parent.top
        anchors.bottom: parent.bottom
        width: parent.width

        // add localhost as the initial screen
        Component.onCompleted: {
            CloudClient.switchProfile()
        }

        // version label
        Version {
            z: 2
        }

        //  config hint text
        Text {
            id: configHintText
            text: screenManager.configHint
            color: "white"
            font.pixelSize: dp(15)
            anchors.bottom: parent.bottom
            anchors.left: parent.left
            anchors.right: parent.right
            anchors.margins: dp(100)
            width: parent.width
            horizontalAlignment: Text.AlignHCenter
            z: 2
        }

        // background header
        Rectangle {
            id: configPageBackgroundHeader
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
            id: configPageBackgroundSeparator
            anchors.top: configPageBackgroundHeader.bottom
            width: parent.width
            height: dp(5)
            color:"#96C13D"
            z: 1
        }

        // log console
        Rectangle {
            id: logConsole
            anchors.top: configPageBackgroundSeparator.bottom
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
                    font {
                        family: consoleFont.name;
                        pixelSize: sample.font.pixelSize * .9
                    }
                }

                menu: Menu {
                    MenuItem {
                        text: "Copy"
                        shortcut: "Ctrl+C"
                        onTriggered: {
                            logConsoleTextArea.copy()
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

        // separator for log console, at the bottom of the log console
        Rectangle {
            id: logConsoleSeparator
            anchors.top: logConsole.bottom
            width: parent.width
            height: logConsole.height > 0 ? dp(5) : 0
            color:"#96C13D"
            z: 3
        }

        // background for config area
        Rectangle {
            id: configPageBackground
            anchors.top: logConsoleSeparator.bottom
            width: parent.width
            anchors.bottom: parent.bottom
            color: applicationWindow.errorView.visible ? errorOverlay.color : "#3F95B8"
            z: 1

            MouseArea {
                anchors.fill: parent
                acceptedButtons: Qt.RightButton
                onClicked: configPageMenu.popup()
            }

            Menu {
                id: configPageMenu

                MenuItem {
                    text: "&Send log"
                    shortcut: "Alt+S"
                    onTriggered: {
                        applicationWindow.logManager.uploadLogFile()
                    }
                }
            }
        }

        // error overlay with mouse trap
        Rectangle {
            id: errorOverlay
            anchors.top: errorMessage.bottom
            width: parent.width
            anchors.bottom: parent.bottom
            color: "#A9A9A9"
            opacity: 0.7
            z: 2
            visible: applicationWindow.errorView.visible

            MouseArea {
                anchors.fill: parent
            }
        }

        // error overlay with mouse trap
        Rectangle {
            id: errorMessage
            anchors.top: logConsoleSeparator.bottom
            width: parent.width
            height: dp(22)
            color: "#FFF1E1"
            visible: applicationWindow.errorView.visible
            z: 1

            // error message
            Text {
                id: errorMessageText
                text: applicationWindow.errorView.message
                color: "#8C4A00"
                font.pixelSize: dp(12)
                anchors.top: parent.top
                anchors.left: parent.left
                anchors.bottom: parent.bottom
                anchors.margins: dp(5)
            }

            // error retry link
            Text {
                id: errorRetryLink
                text: "Retry"
                font.underline: true
                font.pixelSize: errorMessageText.font.pixelSize
                color: errorMessageText.color
                anchors.top: parent.top
                anchors.left: errorMessageText.right
                anchors.margins: errorMessageText.anchors.margins
                visible: !applicationWindow.errorView.retrying

                MouseArea {
                    anchors.fill: parent
                    acceptedButtons: Qt.LeftButton
                    cursorShape: Qt.PointingHandCursor
                    onClicked: {
                        applicationWindow.errorView.retry()
                    }
                }
            }

            // error retrying text
            Text {
                id: errorRetryingText
                text: "Retrying..."
                font.italic: true
                font.pixelSize: errorMessageText.font.pixelSize
                color: errorMessageText.color
                anchors.top: parent.top
                anchors.left: errorMessageText.right
                anchors.margins: errorMessageText.anchors.margins
                visible: applicationWindow.errorView.retrying
            }
        }

        // log upload banner
        Rectangle {
            id: logUploadBanner
            anchors.top: logConsoleSeparator.bottom
            width: parent.width
            height: dp(22)
            color: "#D6E5FF"
            visible: applicationWindow.logManager.dialogVisible
            z: 1

            // log upload message
            Text {
                id: logUploadText
                text: applicationWindow.logManager.dialogText
                color: "#0644A8"
                font.pixelSize: dp(12)
                anchors.top: parent.top
                anchors.left: parent.left
                anchors.bottom: parent.bottom
                anchors.margins: dp(5)
            }

            // log upload url
            Text {
                id: logUploadUrl
                text: applicationWindow.logManager.dialogUrl
                color: "#0644A8"
                font.pixelSize: dp(12)
                anchors.top: parent.top
                anchors.left: logUploadText.right
                anchors.bottom: parent.bottom
                anchors.margins: dp(5)
                visible: applicationWindow.logManager.dialogUrl !== ""

                onLinkActivated: {
                    applicationWindow.logManager.dismissDialog()
                    Qt.openUrlExternally(link)
                }

                MouseArea {
                    anchors.fill: parent
                    cursorShape: Qt.PointingHandCursor
                    acceptedButtons: Qt.NoButton
                }
            }

            //  log upload dismiss link
            Text {
                id: logUploadLink
                text: "Dismiss"
                font.underline: true
                font.pixelSize: logUploadText.font.pixelSize
                color: logUploadText.color
                anchors.top: parent.top
                anchors.left: logUploadUrl.right
                anchors.margins: logUploadText.anchors.margins

                MouseArea {
                    anchors.fill: parent
                    acceptedButtons: Qt.LeftButton
                    cursorShape: Qt.PointingHandCursor
                    onClicked: {
                        applicationWindow.logManager.dismissDialog()
                    }
                }
            }
        }

        // configuration area
        ScrollView {
            id: screenArrangementScrollView
            anchors.top: errorMessage.bottom
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
                        z: errorMessageDialog.visible ? 5 : 1
                        property point beginDrag
                        property int modelIndex: -1

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
                                    color: lastErrorCode === 0 ? "#96C13D" : "red"
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
                                sourceSize.height: dp(14)
                                anchors.horizontalCenter: connectingBar.horizontalCenter
                                anchors.verticalCenter: connectingBar.verticalCenter
                                visible: lastErrorCode !== 0
                                smooth: false
                                source: "qrc:/res/image/error-indication.svg"
                                z: 2
                                property bool errorDialog: false

                                MouseArea {
                                    anchors.fill: parent
                                    acceptedButtons: Qt.LeftButton

                                    onReleased: {
                                        errorIndication.errorDialog = !errorIndication.errorDialog
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
                                visible: errorIndication.errorDialog && errorIndication.visible
                                fillMode: Image.PreserveAspectFit
                                source: "qrc:/res/image/error-message-dialog.png"

                                Image {
                                    id: closeIcon
                                    anchors.top: errorMessageDialog.top
                                    anchors.topMargin: dp(17)
                                    anchors.right: errorMessageDialog.right
                                    anchors.rightMargin: dp(5)
                                    smooth: true
                                    source: "qrc:/res/image/close-icon.svg"
                                    sourceSize.width: dp(7)
                                    sourceSize.height: dp(7)
                                    visible: errorMessageDialog.visible

                                    MouseArea {
                                        anchors.fill: parent
                                        acceptedButtons: Qt.LeftButton

                                        onReleased: {
                                            errorIndication.errorDialog = false
                                        }
                                    }
                                }

                                Text {
                                    id: errorMessageText
                                    width: parent.width - dp(5)
                                    font.pixelSize: dp(10)
                                    anchors.top: errorMessageDialog.top
                                    anchors.left: errorMessageDialog.left
                                    anchors.right: errorMessageDialog.right
                                    anchors.topMargin: dp(30)
                                    anchors.leftMargin: dp(5)
                                    anchors.rightMargin: anchors.leftMargin
                                    horizontalAlignment: Text.AlignHCenter
                                    wrapMode: Text.WordWrap
                                    text: errorMessage
                                    visible: errorMessageDialog.visible
                                }

                                Text {
                                    id: helpLinkText
                                    width: parent.width
                                    font.pixelSize: dp(10)
                                    anchors.top: errorMessageText.bottom
                                    anchors.topMargin: dp(7)
                                    horizontalAlignment: Text.AlignHCenter
                                    wrapMode: Text.WordWrap
                                    text: helpLink
                                    visible: errorMessageDialog.visible

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
