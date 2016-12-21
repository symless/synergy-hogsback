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

		// background header
		Rectangle {
			id: configurationPageBackgroundHeader
			anchors.top: parent.top
			width: parent.width
			height: 70
			color:"white"

			// navigation icon
			Rectangle {
				width: 50
				height: 50
				z: 1
				anchors.left: parent.left
				anchors.leftMargin: 10
				anchors.bottom: parent.bottom
				anchors.bottomMargin: 10
				color: "transparent"

				Image {
					id: navigationImage
					width: parent.width
					height: parent.height
					fillMode: Image.Stretch
					smooth: true
					source: "qrc:/res/image/navigation.png"
				}
			}

			// option icon
			Rectangle {
				id: option
				width: 50
				height: 50
				z: 1
				anchors.right: parent.right
				anchors.rightMargin: 10
				anchors.bottom: parent.bottom
				anchors.bottomMargin: 10
				color: "transparent"

				Image {
					id: optionImage
					width: parent.width
					height: parent.height
					fillMode: Image.Stretch
					smooth: true
					source: "qrc:/res/image/option.png"
				}
			}

			// search icon
			Rectangle {
				id: search
				width: 50
				height: 50
				z: 1
				anchors.right: option.left
				anchors.rightMargin: 10
				anchors.bottom: parent.bottom
				anchors.bottomMargin: 10
				color: "transparent"
				property bool searching: true

				MouseArea {
					id: searchMouseArea
					anchors.fill: parent
					hoverEnabled: true
					onPressed: {
						if (search.searching) {
							searchImage.source = "qrc:/res/image/signal-tower-off.png"
						}
						else {
							searchImage.source = "qrc:/res/image/signal-tower-on.png"
						}

						search.searching = !search.searching
					}
				}

				Image {
					id: searchImage
					width: parent.width
					height: parent.height
					fillMode: Image.Stretch
					smooth: true
					source: "qrc:/res/image/signal-tower-on.png"
				}
			}
		}

		// separator
		Rectangle {
			id: configurationPageBackgroundSeparator
			anchors.top: configurationPageBackgroundHeader.bottom
			width: parent.width
			height: 7
			color:"#96C13D"
		}

		// background
		Rectangle {
			id: configurationPageBackground
			anchors.top: configurationPageBackgroundSeparator.bottom
			width: parent.width
			anchors.bottom: parent.bottom
			color:"#3F95B8"
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
