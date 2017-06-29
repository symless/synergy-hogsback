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

            Version {}

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
                height: dp(45)
                color: "transparent"
                anchors.horizontalCenter: parent.horizontalCenter
                anchors.verticalCenter: parent.verticalCenter

                Rectangle {
                    id: symlessLogin
                    anchors.horizontalCenter: parent.horizontalCenter
                    anchors.top: parent.top
                    width: dp(140)
                    height: dp(35)
                    color: "transparent"

                    Image {
                        id: symlessLoginImage
                        sourceSize.width: parent.width
                        sourceSize.height: sourceSize.width * 67/368
                        smooth: false
                        source: "qrc:/res/image/symless-signin-button.svg"
                    }

                    MouseArea {
                        id: symlessLoginMouseArea
                        anchors.fill: parent
                        hoverEnabled: true
                        cursorShape: Qt.PointingHandCursor

                        onPressed: {
                            if (AppConfig.userToken()) {
                                var url = "https://symless.com/oauth/authorize?"
                                url += "client_id="
                                url += CloudClient.loginClientId()
                                url += "&redirect_uri=" + CloudClient.serverHostname()
                                url += "/login/with-symless"
                                url += "&response_type=code"
                                url += "&state=" + AppConfig.userToken()
                                Qt.openUrlExternally(url)
                                CloudClient.getUserId()
                            } else {
                                hint.text = "Couldn't connect to Synergy Cloud.\nPlease check your Internet connection and then try again."
                                CloudClient.getUserToken()
                            }
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
                color: "white"
                text: ""
                horizontalAlignment: Text.AlignHCenter
                anchors.top: signInArea.bottom
                anchors.horizontalCenter: signInArea.horizontalCenter
            }

            Connections {
                target: CloudClient
                onLoginFail: {
                    hint.text = error
                }
                onInvalidAuth: {
                    hint.text = "Oops! There was a problem.\nPlease try again."
                }
            }
        }
    }
}
