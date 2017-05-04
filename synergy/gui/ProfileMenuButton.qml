import QtQuick 2.0

Item {
    id: profileButtonItem
    height: 32
    width: parent.width
    property int profileId;
    property string profileName;
    property bool clicked: false;
    property bool enable: true;
    property string target;
    signal buttonClicked;

    function buttonEntered() {
        profileButton.state = "hovered";
    }

    function buttonExited() {
        if (clicked == false) {
            profileButton.state = "normal";
        } else {
            profileButton.state = "clicked"
        }
    }

    Rectangle {
        id: profileButton;
        height: parent.height
        width: parent.width

        Rectangle {
            anchors.fill: parent
            anchors.leftMargin: 10
            anchors.rightMargin: 10
            color: "transparent"

            Image {
                id: profileIcon
                source: "qrc:/res/image/profile-icon.svg"
                sourceSize.width: 24
                sourceSize.height: 16
                anchors {
                    verticalCenter: parent.verticalCenter;
                    left: parent.left
                    leftMargin: 5
                }
            }

            TextEdit {
                id: buttonText
                text: qsTr(profileName)
                anchors {
                    verticalCenter: parent.verticalCenter;
                    left: profileIcon.right;
                    leftMargin: 10
                }
                font {
                    family: "Raleway"
                    pointSize: 11.5
                }

                z: 100
            }
        }

        MouseArea {
            hoverEnabled: enable
            enabled: enable
            anchors.fill: parent

            onClicked: {
                profileButtonItem.clicked = true;
                buttonClicked();
            }
            onEntered: buttonEntered();
            onExited: buttonExited();
        }

        states: [
            State {
                name: "clicked";
            },
            State {
                name: "hovered";
                PropertyChanges { target: profileButton; color: "#CBCBCB"; }
            },
            State {
                name: "normal";
            }
        ]
    }
}
