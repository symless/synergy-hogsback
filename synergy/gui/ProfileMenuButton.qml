import QtQuick 2.7

Item {
    id: profileButtonItem
    height: 32
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
            anchors.leftMargin: 10
            anchors.rightMargin: 10
            color: "transparent"
            clip: true

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

            TextInput  {
                id: buttonText
                text: qsTr(profileName)
                z: 100
                anchors {
                    verticalCenter: parent.verticalCenter;
                    left: profileIcon.right;
                    leftMargin: 10
                }
                font {
                    family: "Raleway"
                    pointSize: 11.5
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
