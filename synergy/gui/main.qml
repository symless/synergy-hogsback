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
            AppConfig.save()
            stackView.push({item : Qt.resolvedUrl("ConfigurationPage.qml")})
        }
    }

    onClosing: {
        cloudClient.removeScreen()
    }

    AccessibilityManager {
        id: accessibilityManager
    }

    StackView {
        id: stackView
        anchors.fill: parent
        focus: true

        Keys.onPressed: {
            keyReceived(event.key)
        }

        function nextPage() {
            if (!accessibilityManager.processHasAccessibility()) {
                return {item : Qt.resolvedUrl("AccessibilityPage.qml"), properties: { objectName: "AccessibilityPage"}}
            } else if (cloudClient.verifyUser()) {
                return {item : Qt.resolvedUrl("ConfigurationPage.qml"), properties: { objectName: "ConfigurationPage"}}
            } else {
                return {item : Qt.resolvedUrl("ActivationPage.qml"), properties: { objectName: "ActivationPage"}}
            }
        }

        initialItem: {
            nextPage()
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

