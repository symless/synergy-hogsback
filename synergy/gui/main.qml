import QtQuick 2.3
import QtQuick.Controls 1.2

import com.synergy.gui 1.0

ApplicationWindow {
    id : applicationWindow
    visible: true
    width: 800
    height: 600
    minimumWidth: 800
    minimumHeight: 600
    title: qsTr("Synergy")
    signal keyReceived(int key)
    property alias cloudClient: cloudClient

    CloudClient {
        id: cloudClient
    }

    Connections {
        target: cloudClient
        onLoginOk: {
            stackView.push({item : Qt.resolvedUrl("ConfigurationPage.qml")})
        }
    }

    StackView {
        id: stackView
        anchors.fill: parent
        focus: true

        Keys.onPressed: {
            keyReceived(event.key)
        }

        initialItem: {
            if (cloudClient.verifyUser()) {
                [{item : Qt.resolvedUrl("ConfigurationPage.qml"), properties: { objectName: "ConfigurationPage"}}]
            }
            else {
                [{item : Qt.resolvedUrl("ActivationPage.qml"), properties: { objectName: "ActivationPage"}}]
            }
        }

        onCurrentItemChanged: {
            if (stackView.currentItem) {
                if (stackView.currentItem.objectName == "ActivationPage") {
                    cloudClient.getUserToken()
                }
            }
        }
    }
}

