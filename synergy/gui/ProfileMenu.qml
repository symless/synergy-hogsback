import QtQuick 2.0
import QtQuick.Controls 1.4
import QtQuick.Controls.Styles 1.4

import com.synergy.gui 1.0

Rectangle {
   signal profileCreated (string name)
   id: profileMenuFrame
   color: "#4D4D4D"
   width: 160
   height: childrenRect.height + 2

   Rectangle {
       id: profileMenu
       visible: true
       focus: true
       width: parent.width - 2
       height: childrenRect.height
       anchors {
           margins: 1
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

           ProfileListModel {
               id: profileListModel
           }

           Repeater {
               id: profileListView
               model: profileListModel
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
                        profileListModel.pop()
                   }
               }
           }

           Rectangle {
               id: profileMenuOptions
               height: 50
               width: parent.width
               color: "#4D4D4D"

               Rectangle {
                    color: "white"
                    width: parent.width
                    anchors.bottom: parent.bottom
                    height: parent.height - 1

                    Button {
                       text: "New Profile"
                       anchors.margins: 10
                       anchors.fill: parent
                       style: SynergyButtonStyle {}
                       activeFocusOnPress: true
                       onClicked: {
                           if (profileList.editIndex == -1) {
                                profileList.editIndex = profileListModel.add()
                           }
                       }
                   }
               }
            }
       }
   }
}
