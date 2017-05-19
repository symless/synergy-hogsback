import QtQuick 2.5
import QtQuick.Controls 1.4

Rectangle {
    id: buttonRect
    property color backgroundColour: "#4D4D4D"
    property string buttonText
    property int fontSize: dp(14)
    property bool underlineOnHover: true
    signal buttonClicked

    width: text.width + dp(15)
    height: text.height * 1.7
    color: backgroundColour

    HeaderText {
        id: text
        color: "white"
        anchors.horizontalCenter: parent.horizontalCenter
        anchors.verticalCenter: parent.verticalCenter
        text: buttonText
        font {
            pixelSize: fontSize
            bold: true
            underline: buttonMouseArea.containsMouse & underlineOnHover
        }
    }

    MouseArea {
        id:buttonMouseArea
        anchors.fill: parent
        hoverEnabled: true
        onClicked: {
            buttonClicked()
        }
    }
}
