import QtQuick 2.5

Item {
    id: profileButtonItem
    height: dp(24)
    width: parent.width
    property int profileId;
    property string profileName;
    property bool editFocus: false;
    signal buttonClicked;
    signal editCancelled;
    signal editCompleted;

    Rectangle {
        id: profileButton;
        height: parent.height
        width: parent.width

        Rectangle {
            anchors.fill: parent
            anchors.leftMargin: dp(5)
            anchors.rightMargin: dp(5)
            color: "transparent"
            clip: true

            Image {
                id: profileIcon
                source: "qrc:/res/image/profile-icon.svg"
                sourceSize.width: dp(18)
                sourceSize.height: dp(12)
                smooth: false
                anchors {
                    verticalCenter: parent.verticalCenter;
                    left: parent.left
                    leftMargin: dp(5)
                }
            }

            TextInput  {
                id: buttonText
                text: qsTr(profileName)
                z: 100
                anchors {
                    verticalCenter: parent.verticalCenter;
                    left: profileIcon.right;
                    leftMargin: dp(10)
                }
                font {
                    pixelSize: dp(11.5)
                }
                focus: editFocus
                onEditingFinished: {
                    if (text == "") {
                        editCancelled()
                    } else {
                        profileName = text
                        editCompleted()
                    }

                }
            }
        }

        MouseArea {
            hoverEnabled: true
            anchors.fill: parent
            onClicked: { buttonClicked(); }
        }

        states: [
            State {
                name: "active";
                PropertyChanges { target: profileButton; color: "#CBCBCB"; }
            },
            State {
                name: "inactive";
            }
        ]
    }
}
