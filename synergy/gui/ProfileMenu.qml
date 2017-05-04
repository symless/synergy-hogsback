import QtQuick 2.0
import QtQuick.Controls 1.4
import QtQuick.Controls.Styles 1.4

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

           ProfileMenuButton {
               profileName: "Home"
               profileId: 1
               onButtonClicked: switchProfile(profileId);
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
               height: 45
               width: parent.width
               color: "#4D4D4D"

               Rectangle {
                    color: "white"
                    width: parent.width
                    anchors.bottom: parent.bottom
                    height: parent.height - 1

                    Button {
                       text: "Save"
                       anchors.margins: 10
                       anchors.fill: parent
                       style: SynergyButtonStyle {}
                   }
               }
            }
       }
   }
}
