import QtQuick 2.0
import QtQuick.Controls 1.2

import com.synergy.gui 1.0

Rectangle {

    Item {
        id: activationPage
        anchors.fill: parent

        Rectangle {
            id: rectangle1
            anchors.fill: parent
            color:"#3f95b8"

            Version {
            }

            // background header
            Rectangle {
                id: activationPageBackgroundHeader
                anchors.top: parent.top
                width: parent.width
                height: 94
                color:"white"

                Image {
                    id: logo
                    fillMode :Image.PreserveAspectFit
                    anchors.bottom: parent.bottom
                    anchors.bottomMargin: 15
                    anchors.top: parent.top
                    anchors.topMargin: 15
                    anchors.horizontalCenter: parent.horizontalCenter
                    source: "res/image/synergy-logo.png"
                }
            }

            // separator
            Rectangle {
                id: activationPageBackgroundSeparator
                anchors.top: activationPageBackgroundHeader.bottom
                width: parent.width
                height: 7
                color:"#96C13D"
            }

            Rectangle {
                id: signInArea
                width: 350
                height: 235
                color: "transparent"
                anchors.horizontalCenter: parent.horizontalCenter
                anchors.verticalCenter: parent.verticalCenter

                Rectangle {
                    id: socialLogin
                    anchors.horizontalCenter: parent.horizontalCenter
                    anchors.top: parent.top
                    width: 200
                    height: 50
                    color: "transparent"
                    Image {
                        id: googleImage
                        anchors.fill: parent
                        fillMode: Image.PreserveAspectFit
                        smooth: true
                        source: "qrc:/res/image/google.png"
                    }
                    MouseArea {
                        id: googleMouseArea
                        anchors.fill: parent
                        hoverEnabled: true

                        onPressed: {
                            if (AppConfig.userToken()) {
                                var url = "https://accounts.google.com/o/oauth2/v2/auth?"
                                url += "client_id="
                                url += "735056519324-0rtc3fo39qol3i6c8irloqbgjrdnt4mi.apps.googleusercontent.com"
                                url += '&'
                                url += "redirect_uri="
                                url += "https://alpha1.cloud.symless.com/login/with-google"
                                url += '&'
                                url += "response_type"
                                url += "=code"
                                url += '&'
                                url += "scope="
                                url += "https://www.googleapis.com/auth/userinfo.email"
                                url += '&'
                                url += "state="
                                url += AppConfig.userToken()
                                Qt.openUrlExternally(url)

                                hint.text = ""
                                cloudClient.getUserId()
                            }
                            else {
                                hint.text = "Can't connect with Synergy Cloud. Please check your Internet connection."
                            }
                        }
                    }
                }

                Rectangle {
                    id: orSaparator
                    width: parent.width
                    height: 30
                    anchors.top: socialLogin.bottom
                    anchors.topMargin: 10
                    color: "transparent"

                    Rectangle {
                        border.width: 1
                        height: 2
                        width: parent.width / 2 - 10
                        border.color: "#2d2b19"
                        anchors.left: parent.left
                        anchors.top: parent.top
                        anchors.topMargin: 14
                    }

                    Text {
                        id: orText
                        width: parent.width
                        horizontalAlignment: Text.AlignHCenter
                        anchors.verticalCenter: parent.verticalCenter
                        text: "Or"
                    }

                    Rectangle {
                        border.width: 1
                        height: 2
                        width: parent.width / 2 - 10
                        border.color: "#2d2b19"
                        anchors.right: parent.right
                        anchors.top: parent.top
                        anchors.topMargin: 14
                    }
                }

                TextField {
                    id: email
                    z: 2
                    height: 35
                    width: parent.width
                    anchors.top : orSaparator.bottom
                    anchors.topMargin: 10
                    placeholderText: qsTr("Email")
                    horizontalAlignment: Text.AlignHCenter
                }

                TextField {
                    id: password
                    z: 2
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
                    z: 2
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

                Connections {
                    target: applicationWindow
                    onKeyReceived: {
                        if (key == Qt.Key_Return) {
                            signInButton.clicked();
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
                anchors.top: signInArea.bottom
                anchors.topMargin: 10
                anchors.horizontalCenter: signInArea.horizontalCenter
                font.pixelSize: 10
            }

            Connections {
                target: cloudClient
                onLoginFail: {
                    hint.text = error
                }
            }

            Connections {
                target: cloudClient
                onInvalidAuth: {
                    hint.text = "Invalid credentials. Please login again."
                }
            }
        }
    }
}
