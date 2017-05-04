import QtQuick 2.0
import QtQuick.Controls.Styles 1.4

ButtonStyle {
   label: Component {
        Text {
           color: "white"
           anchors.fill: parent
           horizontalAlignment: Text.AlignHCenter
           verticalAlignment: Text.AlignVCenter
           text: control.text
           font {
               family: "AlternateGotNo3D"
               pointSize: 12
               bold: true
           }
       }
   }

   background: Rectangle {
        implicitWidth: 100
        implicitHeight: 25
        color: control.hovered ? "#5D7CBD" : "#4D4D4D"
   }
}
