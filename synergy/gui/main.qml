import QtQuick 2.5
import QtQuick.Controls 1.4

import com.synergy.gui 1.0

ApplicationWindow {
    id : applicationWindow
    visible: true
    width: dp(600)
    height: dp(450)
    minimumWidth: dp(600)
    minimumHeight: dp(450)
    title: qsTr("Synergy")
    signal keyReceived(int key)
    property ProcessManager processManager: rpcProcessManager

    function dp(v) {
        return v * PixelPerPoint;
    }

    function op(dp) {
        return dp / PixelPerPoint;
    }

    Component.onCompleted: {
        LogManager.setCloudClient(CloudClient)
    }

    Connections {
        target: CloudClient
        onLoginOk: {
            AppConfig.save()
            stackView.push(stackView.nextPage())
        }
    }

    Connections {
        target: CloudClient
        onInvalidAuth: {
            AppConfig.clearAuth()
            stackView.toPage("ActivationPage")
        }
    }

    onClosing: {
        CloudClient.goOffline()
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
            } else if (CloudClient.verifyUser()) {
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
                stackView.currentItem.forceActiveFocus()
                if (stackView.currentItem.objectName == "ActivationPage") {
                    CloudClient.getUserToken()
                }
            }
        }

        function toPage(name) {
            var r = stackView.find(function(item) {
                return item.objectName === name
            })
            if (r) {
                stackView.pop({item : Qt.resolvedUrl(name + ".qml"), properties: { objectName: name}})
            }
            else {
                stackView.push({item : Qt.resolvedUrl(name + ".qml"), properties: { objectName: name}, replace: true})
            }
        }
    }
}
