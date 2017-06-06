import QtQuick 2.5
import QtQuick.Controls 1.4
import QtQuick.Controls.Styles 1.4

import com.synergy.gui 1.0

Rectangle {
    signal profileCreated (string name)
    property ProfileListModel listModel: ProfileManager.listModel()
    id: profileMenu
    color: "#4D4D4D"
    width: dp(160)
    height: childrenRect.height + dp(2)

    Rectangle {
        visible: true
        focus: true
        width: parent.width - dp(2)
        height: childrenRect.height
        anchors {
           margins: dp(1)
           centerIn: parent
        }

        Column {
            id: profileList
            width: parent.width
            property int editIndex: -1;
            anchors {
                top: parent.top
                horizontalCenter: parent.horizontalCenter
            }

            Repeater {
                id: profileListView
                model: profileMenu.listModel
                delegate: ProfileMenuButton {
                    profileName: profName
                    profileId: profId
                    editFocus: profileList.editIndex == index

                    onEditCompleted: {
                        profileList.editIndex = -1
                        profileCreated (profileName)
                    }
                    onEditCancelled: {
                        profileList.editIndex = -1
                        profileMenu.listModel.pop()
                    }
                }
            }

            Rectangle {
                id: profileMenuOptions
                height: dp(50)
                width: parent.width
                color: "#4D4D4D"

                Rectangle {
                    color: "white"
                    width: parent.width
                    anchors.bottom: parent.bottom
                    height: parent.height - dp(1)

                    Button {
                        text: "New Profile"
                        anchors.margins: dp(10)
                        anchors.fill: parent
                        style: SynergyButtonStyle {}
                        activeFocusOnPress: true
                        onClicked: {
                            if (profileList.editIndex == -1) {
                                profileList.editIndex = profileMenu.listModel.add()
                            }
                        }
                    }
                }
            }
        }
    }
}
