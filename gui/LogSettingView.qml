import QtQuick 2.0
import QtQuick.Controls 1.4
import QtQuick.Layouts 1.0
import QtQuick.Dialogs 1.0

Rectangle {
	id: logView
	anchors.fill: parent
	z: 1
	color: "transparent"

	states: [
		State { when: !logView.visible;
			PropertyChanges {
				target: logView
				opacity: 0.0
			}
		}
	]

	transitions: Transition {
		NumberAnimation { property: "opacity"; duration: 700}
	}

	GroupBox {
		id: logGroupBox
		title: "Logging"
		anchors.top: parent.top
		anchors.topMargin: 20
		anchors.left: parent.left
		anchors.leftMargin: 20
		anchors.right: parent.right
		anchors.rightMargin: 20

		ColumnLayout{
			id: logColumnLayout
			anchors.fill: parent

			RowLayout {
				Text {
					id: logLevelText
					anchors.left: parent.left
					text: qsTr("Logging level:")
				}

				ComboBox {
					anchors.left: parent.left
					anchors.leftMargin: 80
					model: [ "Error", "Warning", "Note", "Info", "Debug", "Debug1", "Debug2" ]
				}
			}

			RowLayout {
				anchors.left: parent.left
				anchors.right: parent.right
				anchors.rightMargin: 5

				CheckBox {
					id: logToFileCheckBox
					text: "Log to file:"
				}

				TextField {
					id: logToFileTextInput
					enabled: logToFileCheckBox.checked
					text: fileDialog.cleanPath
					anchors.left: parent.left
					anchors.leftMargin: 80
					anchors.right: logToFileButton.left
					anchors.rightMargin: 10
				}

				Button {
					id: logToFileButton
					enabled: logToFileCheckBox.checked
					text: "Browse.."
					anchors.right: parent.right

					onClicked: {
						fileDialog.open()
					}
				}

				FileDialog {
					id: fileDialog
					visible: false
					modality: Qt.WindowModal
					//selectFolder: true
					selectExisting: false
					title: "Please choose a file"
					property string cleanPath
					onAccepted: {
						var path = fileDialog.fileUrl.toString();
						// remove prefixed "file:///"
						path = path.replace(/^(file:\/{3})/,"");
						// unescape html codes like '%23' for '#'
						cleanPath = decodeURIComponent(path);
								console.log(cleanPath)
						console.log("You chose: " + cleanPath)
					}
				}
			}
		}
	}
}
