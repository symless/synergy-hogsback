import QtQuick 2.5
import QtQuick.Controls 1.4
import QtQuick.Controls.Styles 1.4

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
                height: dp(65)
                color:"white"

                Image {
                    id: logo
                    fillMode :Image.PreserveAspectFit
                    anchors.bottom: parent.bottom
                    anchors.bottomMargin: dp(10)
                    anchors.top: parent.top
                    anchors.topMargin: dp(10)
                    anchors.horizontalCenter: parent.horizontalCenter
                    source: "res/image/synergy-logo.png"
                }
            }

            // separator
            Rectangle {
                id: activationPageBackgroundSeparator
                anchors.top: activationPageBackgroundHeader.bottom
                width: parent.width
                height: dp(5)
                color:"#96C13D"
            }

            Rectangle {
                id: signInArea
                width: dp(245)
                height: dp(165)
                color: "transparent"
                anchors.horizontalCenter: parent.horizontalCenter
                anchors.verticalCenter: parent.verticalCenter

                Rectangle {
                    id: socialLogin
                    anchors.horizontalCenter: parent.horizontalCenter
                    anchors.top: parent.top
                    width: dp(140)
                    height: dp(35)
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
                    height: dp(21)
                    anchors.top: socialLogin.bottom
                    anchors.topMargin: dp(7)
                    color: "transparent"

                    Rectangle {
                        id: leftBar
                        border.width: dp(1)
                        height: dp(1)
                        width: (parent.width - orText.width * 2) / 2
                        border.color: "#2d2b19"
                        anchors.left: parent.left
                        anchors.top: parent.top
                        anchors.topMargin: dp(10)
                        color: border.color
                    }

                    BodyText {
                        id: orText
                        horizontalAlignment: Text.AlignHCenter
                        anchors.horizontalCenter: parent.horizontalCenter
                        anchors.verticalCenter: parent.verticalCenter
                        text: "Or"
                    }

                    Rectangle {
                        border.width: leftBar.border.width
                        height: leftBar.height
                        width: leftBar.width
                        border.color: leftBar.border.color
                        anchors.right: parent.right
                        anchors.top: parent.top
                        anchors.topMargin: leftBar.anchors.topMargin
                        color: border.color
                    }
                }

                TextField {
                    id: email
                    z: 2
                    height: dp(25)
                    width: parent.width
                    anchors.top : orSaparator.bottom
                    anchors.topMargin: dp(7)
                    placeholderText: qsTr("Email")
                    horizontalAlignment: Text.AlignHCenter
                    style: TextFieldStyle {
                        background: Rectangle { color: "white" }
                    }
                }

                TextField {
                    id: password
                    z: 2
                    height: email.height
                    width: parent.width
                    anchors.top : email.bottom
                    anchors.topMargin: email.anchors.topMargin
                    placeholderText: qsTr("Password")
                    horizontalAlignment: Text.AlignHCenter
                    echoMode: TextInput.Password
                    style: TextFieldStyle {
                        background: Rectangle { color: "white" }
                    }
                }

                Button {
                    id: signInButton
                    z: 2
                    height: email.height
                    width: parent.width
                    anchors.top : password.bottom
                    anchors.topMargin: email.anchors.topMargin
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

            BodyText {
                id: hint
                z: 1
                color: "White"
                horizontalAlignment: Text.AlignHCenter
                anchors.top: signInArea.bottom
                anchors.topMargin: email.anchors.topMargin
                anchors.horizontalCenter: signInArea.horizontalCenter
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
