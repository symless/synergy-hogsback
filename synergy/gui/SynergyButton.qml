import QtQuick 2.5
import QtQuick.Controls 1.4
import QtQuick.Controls.Styles 1.4

Rectangle {
    property color backgroundColour: "#4D4D4D"
    property string buttonText
    property int fontSize: 14
    property bool underlineOnHover: true
    signal buttonClicked

    width: childrenRect.width + 20
    height: childrenRect.height + 10
    color: backgroundColour

    Button {
        text: buttonText
        anchors.horizontalCenter: parent.horizontalCenter
        anchors.verticalCenter: parent.verticalCenter
        style: ButtonStyle {
            label: Component {
                Text {
                    color: "white"
                    anchors.fill: parent
                    horizontalAlignment: Text.AlignHCenter
                    verticalAlignment: Text.AlignVCenter
                    text: control.text
                    renderType: Text.NativeRendering
                    font {
                        family: "AlternateGotNo3D"
                        pointSize: fontSize
                        bold: true
                        underline: control.hovered && underlineOnHover
                    }
                }
            }

            background: Rectangle {
                anchors.fill: parent
                color: backgroundColour
            }
        }

        onClicked: {
            buttonClicked()
        }
    }
}
