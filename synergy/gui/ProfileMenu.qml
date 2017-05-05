import QtQuick 2.0
import QtQuick.Controls 1.4
import QtQuick.Controls.Styles 1.4

import com.synergy.gui 1.0

Rectangle {
   id: profileMenuFrame
   color: "#4D4D4D"
   width: 160
   height: childrenRect.height + 2

   Rectangle {
       id: profileMenu
       visible: isOpen
       focus: isOpen
       width: parent.width - 2
       height: childrenRect.height
       anchors {
           margins: 1
           centerIn: parent
       }
       property bool isOpen: true
       signal switchProfile (int profileId)

       Column {
           id: profileList
           width: parent.width
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
                   profileName: profile.name
                   profileId: profile.id
               }
           }

           ProfileMenuButton {
               profileName: "Work"
               profileId: 2
               onButtonClicked: switchProfile(profileId);
               clicked: true
           }

           ProfileMenuButton {
               profileName: "Big Momma's House"
               profileId: 2
               onButtonClicked: switchProfile(profileId);
               clicked: true
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
                   }
               }
            }
       }
   }
}
