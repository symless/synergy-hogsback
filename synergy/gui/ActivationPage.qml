import QtQuick 2.0
import QtQuick.Controls 1.2

import com.synergy.gui 1.0

Rectangle {

    Item {
        id: activationPage
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

            Rectangle {
                id: socialLogin
                anchors.left: signIn.left
                anchors.top: signIn.bottom
                anchors.topMargin: 10
                width: 35
                height: 35

                Image {
                    id: facebookImage
                    anchors.fill: parent
                    fillMode: Image.Stretch
                    smooth: true
                    source: "qrc:/res/image/google.png"
                }
                MouseArea {
                    id: facebookMouseArea
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
                            console.log(url)
                            Qt.openUrlExternally(url)
                            //Qt.openUrlExternally("https://accounts.google.com/o/oauth2/v2/auth?client_id=735056519324-0rtc3fo39qol3i6c8irloqbgjrdnt4mi.apps.googleusercontent.com&redirect_uri=https://alpha1.cloud.symless.com/login/with-google&response_type=code&scope=https://www.googleapis.com/auth/userinfo.email")
                        }
                        else {
                            hint.text = "Can't connect with Synergy Cloud. Please check your internet."
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
