import QtQuick 2.0
import QtQuick.Controls 1.4
import QtQuick.Layouts 1.0

Rectangle {
	id: configView
	anchors.left: parent.left
	anchors.leftMargin: 10
	anchors.right: parent.right
	anchors.rightMargin: 10
	anchors.top: parent.top
	anchors.bottom: parent.bottom

	z: 1
	color: "transparent"

	states: [
		State { when: !configView.visible;
			PropertyChanges {
				target: configView
				opacity: 0.0
			}
		}
	]

	transitions: Transition {
		NumberAnimation { property: "opacity"; duration: 700}
	}

	ExclusiveGroup { id: configurationGroup }
	RadioButton {
		id: autoConfigurationRadioButton
		text: "Auto Configuration"
		checked: true
		exclusiveGroup: configurationGroup
	}

	RadioButton {
		id: manualConfigurationRadioButton
		anchors.top: autoConfigurationRadioButton.bottom
		anchors.topMargin: 10
		text: "Manual Configuration"
		exclusiveGroup: configurationGroup
	}

	Rectangle {
		id: screenListBackground
		anchors.left: parent.left
		anchors.right: parent.right
		anchors.top: manualConfigurationRadioButton.bottom
		anchors.topMargin: 10
		anchors.bottom: parent.bottom
		width: 250
		color: "transparent"
		radius: 3
		enabled: manualConfigurationRadioButton.checked

		Rectangle {
			id: screenListRectangle
			width: 140
			anchors.top: parent.top
			anchors.bottom: parent.bottom
			anchors.left: parent.left
			color: "#66666666"

			ListView {
				id: screenList
				anchors.fill: parent
				clip: true

				delegate: Item {
					width: parent.width
					height: 30

					Rectangle {
						anchors.fill: parent
						color: "transparent"

						MouseArea {
							id: mouseArea
							anchors.fill: parent
							hoverEnabled: true

							onClicked: {
								screenList.currentIndex = index
							}
						}

						CheckBox {
							id: screenCheckBox
							anchors.left: parent.left
							anchors.leftMargin: 5
							anchors.verticalCenter: parent.verticalCenter
							checked: active
						}

						Rectangle {
							id: itemName
							height: parent.height
							anchors.left: screenCheckBox.right
							anchors.right: parent.right
							color: "transparent"

							Text {
								text: name
								anchors.verticalCenter: parent.verticalCenter
								anchors.horizontalCenter: parent.horizontalCenter
								font.pixelSize: 12
								font.family: "Tahoma"
								color: enabled ? "black" : "gray"
							}
						}
					}
				}

				highlight: Rectangle {
					anchors.margins: 1
					opacity: 1
					color: "gray"
				}

				model: ListModel {

					ListElement {
						name: "Screen1"
						active: true
					}

					ListElement {
						name: "Screen2"
						active: true
					}

					ListElement {
						name: "Screen3"
						active: true
					}
				}
			}
		}

		// screen View
		Rectangle {
			id: settingsView
			anchors.top: screenListRectangle.top
			anchors.left: screenListRectangle.right
			anchors.leftMargin: 5
			anchors.right: parent.right
			anchors.bottom: parent.bottom
			color: "#66666666"
			visible: true
			radius: 3

			GroupBox {
				id: modifierKeysGroupBox
				title: "Modifier keys"
				anchors.left: parent.left
				anchors.leftMargin: 5
				anchors.top: parent.top
				anchors.topMargin: 5

				ColumnLayout{
					RowLayout {
						anchors.left: parent.left
						anchors.right: parent.right
						anchors.rightMargin: 5

						Label {
							id: shiftKey
							text: "Shift:"
						}

						ComboBox {
							id: shiftKeyComboBox
							anchors.left: parent.left
							anchors.right: parent.right
							anchors.leftMargin: 50
							model: [ "Shift", "Ctrl", "Alt", "Meta", "Super" ]
						}
					}

					RowLayout {
						anchors.left: parent.left
						anchors.right: parent.right
						anchors.rightMargin: 5

						Label {
							id: ctrlKey
							text: "Ctrl:"
						}

						ComboBox {
							id: ctrlKeyComboBox
							anchors.left: parent.left
							anchors.right: parent.right
							anchors.leftMargin: 50
							model: [ "Shift", "Ctrl", "Alt", "Meta", "Super" ]
						}
					}

					RowLayout {
						anchors.left: parent.left
						anchors.right: parent.right
						anchors.rightMargin: 5

						Label {
							id: altKey
							text: "Alt:"
						}

						ComboBox {
							id: altKeyComboBox
							anchors.left: parent.left
							anchors.right: parent.right
							anchors.leftMargin: 50
							model: [ "Shift", "Ctrl", "Alt", "Meta", "Super" ]
						}
					}

					RowLayout {
						anchors.left: parent.left
						anchors.right: parent.right
						anchors.rightMargin: 5

						Label {
							id: metaKey
							text: "Meta:"
						}

						ComboBox {
							id: metaKeyComboBox
							anchors.left: parent.left
							anchors.right: parent.right
							anchors.leftMargin: 50
							model: [ "Shift", "Ctrl", "Alt", "Meta", "Super" ]
						}
					}

					RowLayout {
						anchors.left: parent.left
						anchors.right: parent.right
						anchors.rightMargin: 5

						Label {
							id: superKey
							text: "Super:"
						}

						ComboBox {
							id: superKeyComboBox
							anchors.left: parent.left
							anchors.right: parent.right
							anchors.leftMargin: 50
							model: [ "Shift", "Ctrl", "Alt", "Meta", "Super" ]
						}
					}
				}
			}

			GroupBox {
				title: "Fixes"
				anchors.left: modifierKeysGroupBox.right
				anchors.leftMargin: 5
				anchors.top: parent.top
				anchors.topMargin: 5

				ColumnLayout{
					CheckBox {
						text: "Fix CAPS LOCK key"
					}
					CheckBox {
						text: "Fix NUM LOCK key"
					}
					CheckBox {
						text: "Fix SCROLL LOCK key"
					}
					CheckBox {
						text: "Fix XTest for Xinerama"
					}
				}
			}
		}
	}
}
