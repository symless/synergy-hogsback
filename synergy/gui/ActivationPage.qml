import QtQuick 2.0
import QtQuick.Controls 1.2

import com.synergy.gui 1.0

Rectangle {

    CloudClient {
        id: cloudClient
    }

    Connections {
        target: cloudClient
        onConnected: {
            stackView.push({item : Qt.resolvedUrl("ConfigurationPage.qml")})
        }
    }

    Item {
        id: advancedPage
        anchors.fill: parent

        Rectangle {
            anchors.fill: parent
            color:"#3f95b8"

            Rectangle {
                id: signIn
                width: 350
                height: 185
                color: "transparent"
                anchors.horizontalCenter: parent.horizontalCenter
                anchors.verticalCenter: parent.verticalCenter

                Text {
                    id: signInText
                    z: 1
                    color: "White"
                    text: "Sign in"
                    font.family: "Tahoma"
                    font.bold: false
                    horizontalAlignment: Text.AlignHCenter
                    anchors.top: parent.top
                    anchors.topMargin: 10
                    anchors.horizontalCenter: parent.horizontalCenter
                    font.pixelSize: 30
                }

                TextField {
                    id: email
                    z: 1
                    height: 35
                    width: parent.width
                    anchors.top : signInText.bottom
                    anchors.topMargin: 10
                    placeholderText: qsTr("Email")
                    horizontalAlignment: Text.AlignHCenter
                }

                TextField {
                    id: password
                    z: 1
                    height: 35
                    width: parent.width
                    anchors.top : email.bottom
                    anchors.topMargin: 10
                    placeholderText: qsTr("Password")
                    horizontalAlignment: Text.AlignHCenter
                    echoMode: TextInput.Password
                }

                Button {
                    id: signInButton
                    z: 1
                    height: 35
                    width: parent.width
                    anchors.top : password.bottom
                    anchors.topMargin: 10
                    text: "Sign in"
                    onClicked: {
                        if (email.text && password.text) {
                            hint.text = ""

                            // contact Cloud
                            cloudClient.login(email.text, password.text)
                        }
                        else {
                            hint.text = "Empty email or password"
                        }
                    }
                }
            }

            Text {
                id: hint
                z: 1
                color: "White"
                font.family: "Tahoma"
                font.bold: false
                horizontalAlignment: Text.AlignHCenter
                anchors.top: signIn.bottom
                anchors.topMargin: 10
                anchors.horizontalCenter: signIn.horizontalCenter
                font.pixelSize: 10
            }
        }
    }
}
