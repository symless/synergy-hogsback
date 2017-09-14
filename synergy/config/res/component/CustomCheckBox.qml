import QtQuick 2.5
import QtQuick.Controls 1.4
import QtQuick.Controls.Styles 1.4

CheckBox {
	text: ""
	style: CheckBoxStyle {
		indicator: Rectangle {
			implicitWidth: 14
			implicitHeight: 14
			radius: 2
			border.color: control.activeFocus ? "lightgrey" : "grey"
			border.width: 1
			Rectangle {
				visible: control.checked
				color: "#555"
				border.color: "#333"
				radius: 1
				anchors.margins: 3
				anchors.fill: parent
			}
		}
	}
}
