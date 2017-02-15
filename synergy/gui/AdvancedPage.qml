import QtQuick 2.0
import QtQuick.Controls 1.2
import QtQuick.Controls.Styles 1.2
import QtGraphicalEffects 1.0

Rectangle {
	Item {
		id: advancedPage
		anchors.fill: parent

		// background
		Image {
			id: advancedPageBackground
			anchors.left: advancedPageBackgroundLeft.right
			anchors.right: parent.right
			anchors.top: parent.top
			anchors.bottom: parent.bottom
			source: "qrc:/res/image/background2.png"
		}

		// background left
		Image {
			id: advancedPageBackgroundLeft
			width: settingsListBackground.width
			height: parent.height
			anchors.top: parent.top
			source: "qrc:/res/image/background2_left.png"
		}

		// ok button
		Button {
			id: okButton
			width: 70
			text: "Ok"
			anchors.bottom: parent.bottom
			anchors.bottomMargin: 15
			anchors.right: cancelButton.left
			anchors.rightMargin: 20

			onClicked: {
				stackView.pop();
			}
		}


		// cancel button
		Button {
			id: cancelButton
			width: 70
			text: "Cancel"
			anchors.bottom: okButton.bottom
			anchors.right: parent.right
			anchors.rightMargin: 20

			onClicked: {
				stackView.pop();
			}
		}

		// profile
		Rectangle {
			id: profile
			width: 65
			height: 65
			anchors.top: search.top
			anchors.horizontalCenter: settingsListBackground.horizontalCenter
			color: "transparent"

			Image {
				anchors.fill: parent
				fillMode: Image.PreserveAspectFit
				smooth: true
				source: "qrc:/res/image/pro_user.png"
			}
		}

		Text {
			id: profileDetail
			anchors.horizontalCenter: profile.horizontalCenter
			anchors.top: profile.bottom
			text: "Pro User"
			font.pixelSize: 15
			font.family: "Tahoma"
		}

		// search
		Rectangle {
			id: search
			x: 470
			y: 30
			width: 150
			height: 20
			anchors.top: parent.top
			anchors.topMargin: 15
			anchors.right: parent.right
			anchors.rightMargin: 20
			border.width: 1
			border.color: "darkgrey"
			radius: 3

			Image {
				id: searchImage
				height: parent.height
				anchors.left: parent.left
				fillMode: Image.PreserveAspectFit
				smooth: true
				source: "qrc:/res/image/search.png"
			}

			TextField {
				id: searchInput
				height: parent.height - 2
				anchors.verticalCenter: parent.verticalCenter
				placeholderText: "Search"
				anchors.left: searchImage.right
				verticalAlignment: Text.AlignVCenter
				font.pixelSize: 12

				style: TextFieldStyle {
					background: Rectangle {
						border.width: 0
					}
				}
			}
		}

		// setting list
		Rectangle {
			id: settingsListBackground
			width: 140
			anchors.top: profileDetail.bottom
			anchors.topMargin: 45
			anchors.bottom: parent.bottom
			anchors.left: parent.left
			color: "transparent"
			radius: 3

			ListView {
				id: settingsList
				anchors.fill: parent
				clip: true

				delegate: Item {
					width: parent.width
					height: 50

					Rectangle {
						anchors.fill: parent
						color: "transparent"

						MouseArea {
							id: mouseArea
							anchors.fill: parent
							hoverEnabled: true

							onClicked: {
								settingsList.currentIndex = index
								hoverHighlight.visible = false
							}

							onEntered: {
								hoverHighlight.visible = true
							}

							onExited: {
								hoverHighlight.visible = false
							}
						}

						Image {
							id: hoverHighlight
							anchors.fill: parent
							visible: false
							opacity: 0.3
							scale: 2.5
							source: "qrc:/res/image/highlight.png"
						}

						Image {
							id: itemImage
							width: parent.height - 10
							height: parent.height - 10
							anchors.verticalCenter: parent.verticalCenter
							anchors.left: parent.left
							anchors.leftMargin: 10
							fillMode: Image.PreserveAspectFit
							antialiasing: true
							source: imageSource
							scale: settingsList.currentIndex == index ? 1 : mouseArea.containsMouse ? 0.9 : 0.7
						}

						Rectangle {
							id: itemName
							height: parent.height
							anchors.left: itemImage.right
							anchors.right: parent.right
							color: "transparent"

							Text {
								text: name
								anchors.verticalCenter: parent.verticalCenter
								anchors.horizontalCenter: parent.horizontalCenter
								font.pixelSize: 12
								font.family: "Tahoma"
							}
						}
					}
				}

				highlight: Component {
					Image {
						scale: 2.5
						opacity: 0.7
						source: "qrc:/res/image/highlight.png"
					}
				}

				model: ListModel {

					ListElement {
						name: "General"
						imageSource: "qrc:/res/image/general.png"
					}

					ListElement {
						name: "Config"
						imageSource: "qrc:/res/image/configuration.png"
					}

					ListElement {
						name: "Log"
						imageSource: "qrc:/res/image/log.png"
					}

					ListElement {
						name: "SSL"
						imageSource: "qrc:/res/image/ssl.png"
					}

					ListElement {
						name: "Info"
						imageSource: "qrc:/res/image/info.png"
					}
				}
			}
		}

		// settings View
		Rectangle {
			id: settingsView
			color: "transparent"
			anchors.top: profileDetail.bottom
			anchors.left: settingsListBackground.right
			anchors.leftMargin: 5
			anchors.right: parent.right
			anchors.rightMargin: 20
			anchors.bottom: parent.bottom
			anchors.bottomMargin: 50
			visible: true
			radius: 3

			GeneralSettingView {
				visible: settingsList.currentIndex === 0
			}

			ConfigSettingView {
				visible: settingsList.currentIndex === 1
			}

			LogSettingView {
				visible: settingsList.currentIndex === 2
			}
		}
	}
}
