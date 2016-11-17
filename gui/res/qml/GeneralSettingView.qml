import QtQuick 2.0
import QtQuick.Controls 1.4
import QtQuick.Layouts 1.0

Rectangle {
	id: generalView
	anchors.fill: parent
	z: 1
	color: "transparent"

	states: [
		State { when: !generalView.visible;
			PropertyChanges {
				target: generalView
				opacity: 0.0
			}
		}
	]

	transitions: Transition {
		NumberAnimation { property: "opacity"; duration: 700}
	}

	GroupBox {
		id: networkGroupBox
		title: "Network"
		anchors.top: parent.top
		anchors.topMargin: 20
		anchors.left: parent.left
		anchors.leftMargin: 20
		anchors.right: parent.right
		anchors.rightMargin: 20

		ColumnLayout{
			anchors.fill: parent

			RowLayout {
				anchors.left: parent.left
				anchors.right: parent.right
				anchors.rightMargin: 5

				Label {
					id: localIP
					text: "Local IP:"
				}

				ComboBox {
					id: localIPComboBox
					anchors.left: parent.left
					anchors.leftMargin: 80
					model: [ "10.0.0.39", "192.168.1.1" ]
				}
			}

			RowLayout {
				anchors.left: parent.left
				anchors.right: parent.right
				anchors.rightMargin: 5

				Label {
					id: port
					text: "Port:"
				}

				TextField {
					anchors.left: parent.left
					anchors.leftMargin: 80
					text: "24800"
				}
			}

			RowLayout {
				anchors.left: parent.left
				anchors.right: parent.right
				anchors.rightMargin: 5

				Label {
					id: group
					text: "Group:"
				}

				SpinBox {
					id: groupSpinBox
					anchors.left: parent.left
					anchors.leftMargin: 80
					maximumValue: 255
					value: 0
				}
			}

			CheckBox {
				text: "Use static group"
			}

		}
	}

	GroupBox {
		id: optionsGroupBox
		title: "Options"
		anchors.top: networkGroupBox.bottom
		anchors.topMargin: 20
		anchors.left: parent.left
		anchors.leftMargin: 20
		anchors.right: parent.right
		anchors.rightMargin: 20

		ColumnLayout{
			id: optionsColumnLayout
			anchors.fill: parent

			CheckBox {
				text: "Elevate mode"
			}

			CheckBox {
				text: "Use relative mouse moves"
			}

			CheckBox {
				text: "Synchronize screen savers"
			}

			CheckBox {
				text: "Don't take foreground window on Windows servers"
			}

			CheckBox {
				text: "Enable file drag and drop"
			}

			RowLayout {
				anchors.left: parent.left
				anchors.right: parent.right
				anchors.rightMargin: 5

				CheckBox {
					id: checkClientsInterval
					text: "Check clients every"
				}

				SpinBox {
					maximumValue: 99999
					value: 5000
					anchors.right: checkClientsIntervalUnit.left
					anchors.rightMargin: 10
					enabled: checkClientsInterval.checked
				}

				Text {
					id: checkClientsIntervalUnit
					anchors.right: parent.right
					text: "ms"
					Layout.alignment: Qt.AlignRight | Qt.AlignVCenter
				}
			}

			RowLayout {
				anchors.left: parent.left
				anchors.right: parent.right
				anchors.rightMargin: 5

				CheckBox {
					id: switchAfterWaiting
					text: "Switch after waiting"
				}

				SpinBox {
					maximumValue: 99999
					value: 250
					anchors.right: switchAfterWaitingUnit.left
					anchors.rightMargin: 10
					enabled: switchAfterWaiting.checked
				}

				Text {
					id: switchAfterWaitingUnit
					anchors.right: parent.right
					text: "ms"
					Layout.alignment: Qt.AlignRight | Qt.AlignVCenter
				}
			}

			RowLayout {
				anchors.left: parent.left
				anchors.right: parent.right
				anchors.rightMargin: 5

				CheckBox {
					id: switchDoubleTap
					text: "Switch on double tap within"
				}

				SpinBox {
					maximumValue: 99999
					value: 250
					anchors.right: switchDoubleTapUnit.left
					anchors.rightMargin: 10
					enabled: switchDoubleTap.checked
				}

				Text {
					id: switchDoubleTapUnit
					anchors.right: parent.right
					text: "ms"
					Layout.alignment: Qt.AlignRight | Qt.AlignVCenter
				}
			}
		}
	}

	GroupBox {
		title: "Dead corners"
		anchors.top: optionsGroupBox.bottom
		anchors.topMargin: 20
		anchors.left: parent.left
		anchors.leftMargin: 20
		anchors.right: parent.right
		anchors.rightMargin: 20

		RowLayout {
			anchors.left: parent.left
			anchors.right: parent.right
			anchors.rightMargin: 5

			CheckBox {
				id: tl
				text: "Top-left"
			}

			CheckBox {
				id: tr
				text: "Top-right"
			}

			CheckBox {
				id: bl
				text: "bottom-left"
			}

			CheckBox {
				id: br
				text: "bottom-right"
			}

			SpinBox {
				anchors.right: deadCornerUnit.left
				anchors.rightMargin: 10
				maximumValue: 999
				value: 0
				enabled: tl.checked || tr.checked || bl.checked || br.checked
			}

			Text {
				id: deadCornerUnit
				anchors.right: parent.right
				anchors.rightMargin: 3
				text: "px"
				Layout.alignment: Qt.AlignRight | Qt.AlignVCenter
			}
		}
	}
}

