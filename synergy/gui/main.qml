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

    StackView {
        id: stackView
        anchors.fill: parent
        focus: true

        Keys.onPressed: {
            keyReceived(event.key)
        }

        initialItem: {
            if (AppConfig.userToken() && AppConfig.userId() !== -1) {
                [{item : Qt.resolvedUrl("ConfigurationPage.qml")}]
            }
            else {
                [{item : Qt.resolvedUrl("ActivationPage.qml")}]
            }
        }
    }
}

