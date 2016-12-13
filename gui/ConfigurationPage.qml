import QtQuick 2.0
import QtQuick.Controls 1.2

import com.synergy.gui 1.0

Rectangle {

	Hostname {
		id: localHostname
	}

	ScreenModel {
		id: screenModel
	}

//	IpcClient {
//		id: ipcClient
//	}

	ProcessManager {
		id: processManager
		//ipcClient: ipcClient;
	}

	ScreenManager {
		id: screenManager
		screenModel: screenModel
		processManager: processManager
		viewWidth: screenArrangementScrollView.width
		viewHeight: screenArrangementScrollView.height
	}

	Item {
		id: configurationPage
		anchors.fill: parent

		// background
		Rectangle {
			id: configurationPageBackground
			anchors.top: parent.top
			width: parent.width
			anchors.bottom: configurationPageBackgroundSeparator.top
			color:"#3F95B8"
		}

		// separator
		Rectangle {
			id: configurationPageBackgroundSeparator
			anchors.bottom: configurationPageBackgroundFooter.top
			width: parent.width
			height: 7
			color:"#96C13D"
		}


		// background footer
		Rectangle {
			id: configurationPageBackgroundFooter
			anchors.bottom: parent.bottom
			width: parent.width
			height: 70
			color:"white"
		}

		// hostname
		Text {
			id: hostname
			z: 1
			text: localHostname.hostname()
			font.family: "Tahoma"
			font.bold: false
			verticalAlignment: Text.AlignVCenter
			horizontalAlignment: Text.AlignHCenter
			anchors.top: parent.top
			anchors.topMargin: 8
			anchors.horizontalCenter: parent.horizontalCenter
			font.pixelSize: 15
		}

		// advanced settings
		Rectangle {
			width: 30
			height: 45
			z: 1
			anchors.left: parent.left
			anchors.leftMargin: parent.width / 8 * 3
			anchors.bottom: parent.bottom
			anchors.bottomMargin: 10
			color: "transparent"

			MouseArea {
				id: cogMouseArea
				anchors.fill: parent
				hoverEnabled: true
				onPressed: {
					stackView.push(Qt.resolvedUrl("AdvancedPage.qml"));
				}
			}

			Image {
				id: cogImage
				width: parent.width
				height: parent.height - advancedText.height
				anchors.bottomMargin: 5
				fillMode: Image.Stretch
				smooth: true
				opacity: cogMouseArea.containsMouse ? 1.0 : 0.7
				source: "qrc:/res/image/cog.png"
			}

			Text {
				id: advancedText
				height: 15
				anchors.top: cogImage.bottom
				anchors.topMargin: 0
				anchors.horizontalCenter: parent.horizontalCenter
				text: qsTr("Advanced")
				font.family: "Tahoma"
				font.pixelSize: 15
			}
		}

		// apply button
		Rectangle {
			width: 30
			height: 45
			z: 1
			anchors.right: parent.right
			anchors.rightMargin: parent.width / 8 * 3
			anchors.bottom: parent.bottom
			anchors.bottomMargin: 10
			color: "transparent"

			MouseArea {
				id: applyMouseArea
				anchors.fill: parent
				hoverEnabled: true
				onPressed: {
					screenManager.printBoundingBoxInfo()
					//screenManager.updateConfigFile()
					//processManager.start(modeSwitch.checked)
				}
			}

			Image {
				id: applyImage
				width: parent.width
				height: parent.height - applyText.height
				anchors.horizontalCenter: parent.horizontalCenter
				fillMode: Image.Stretch
				opacity: applyMouseArea.containsMouse ? 1.0 : 0.7
				smooth: true
				source: "qrc:/res/image/apply.png"
			}

			Text {
				id: applyText
				height: 15
				anchors.top: applyImage.bottom
				anchors.topMargin: 0
				anchors.horizontalCenter: parent.horizontalCenter
				text: qsTr("Apply")
				font.family: "Tahoma"
				font.pixelSize: 15
			}
		}

		// server configuration
		ScrollView {
			id: screenArrangementScrollView
			anchors.fill: parent

			states: [
				State { when: !screenArrangementScrollView.visible;
					PropertyChanges {
						target: screenArrangementScrollView
						opacity: 0.0
					}
				}
			]

			transitions: Transition {
				NumberAnimation { property: "opacity"; duration: 700}
			}

			Item {
				id: screenArrangement
				anchors.fill: parent
				scale: screenModel.scale

				Repeater {
					model: screenModel
					Item {
						id: screenIcon
						x: posX; y: posY
						width: screenModel.screenIconWidth()
						height: screenModel.screenIconHeight()
						property point beginDrag
						property int modelIndex

						MouseArea {
							id: mouse
							anchors.fill: parent
							drag.target: screenIcon
							drag.axis: Drag.XandYAxis
							onPressed: {
								beginDrag = Qt.point(screenIcon.x,
												screenIcon.y);
								modelIndex = screenManager.getModelIndex(
												screenIcon.x + 45,
												screenIcon.y + 45)
							}
							onReleased: {
								screenManager.moveModel(modelIndex,
												screenIcon.x - beginDrag.x,
												screenIcon.y - beginDrag.y)
							}
						}

						Image {
							id: itemImage
							parent: screenIcon
							anchors.fill: parent
							fillMode: Image.Stretch
							smooth: true
							source: "qrc:/res/image/screen-inactive.png"

							Text {
								width: parent.width - 20
								anchors.top: parent.top
								anchors.topMargin: 7
								anchors.horizontalCenter: parent.horizontalCenter
								text: name
								font.pixelSize: 15
								minimumPixelSize: 15
								color: "white"
								font.family: "Tahoma"
								fontSizeMode: Text.HorizontalFit
								horizontalAlignment: Text.AlignHCenter
								elide: Text.ElideRight
							}
						}
					}
				}
			}
		}
	}
}
