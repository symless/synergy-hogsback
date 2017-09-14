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
               pixelSize: dp(10)
               bold: false
           }
       }
   }

   background: Rectangle {
        implicitWidth: dp(100)
        implicitHeight: dp(25)
        color: "#4D4D4D"
   }
}
