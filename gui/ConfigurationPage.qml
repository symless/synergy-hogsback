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
		anchors.top: parent.top
		anchors.bottom: parent.bottom
		anchors.left: navigationMenu.right
		width: parent.width

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

				MouseArea {
					id: navigationMouseArea
					anchors.fill: parent
					hoverEnabled: true
					onPressed: {
						if (navigationMenu.opened) {
							navigationMenu.x = -navigationMenu.width
						}
						else {
							navigationMenu.x = 0
						}

						navigationMenu.opened = !navigationMenu.opened
					}
				}
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
				width: 30
				height: 30
				z: 1
				anchors.right: parent.right
				anchors.rightMargin: 20
				anchors.bottom: parent.bottom
				anchors.bottomMargin: 20
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
							searchImage.source =
									"qrc:/res/image/signal-tower-off.png"
						}
						else {
							searchImage.source =
									"qrc:/res/image/signal-tower-on.png"
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
			anchors.topMargin: 15
			anchors.horizontalCenter: parent.horizontalCenter
			font.pixelSize: 30
		}

		// server configuration
		ScrollView {
			id: screenArrangementScrollView
			anchors.top: configurationPageBackgroundSeparator.bottom
			anchors.bottom: parent.bottom
			width: parent.width

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

	// navigation menu
	Rectangle {
		id: navigationMenu
		height: parent.height
		width: 150
		x: -width
		z: 2
		color:"white"
		property bool opened: false

		Behavior on x {NumberAnimation
						{duration: 500; easing.type: Easing.OutQuad}}

		// profile icon
		Rectangle {
			id: profile
			width: parent.width
			height: width
			z: 3
			anchors.horizontalCenter: parent.horizontalCenter
			anchors.top: parent.top
			anchors.topMargin: 20
			color: "transparent"

			Image {
				id: profileImage
				width: parent.width
				height: parent.height
				fillMode: Image.Stretch
				smooth: true
				source: "qrc:/res/image/pro_user.png"
			}
		}

		// configure icon
		Rectangle {
			id: configure
			width: 50
			height: 50
			z: 3
			anchors.horizontalCenter: parent.horizontalCenter
			anchors.top: profile.bottom
			anchors.topMargin: 30
			color: "transparent"

			Image {
				id: configureImage
				width: parent.width
				height: parent.height
				fillMode: Image.Stretch
				smooth: true
				source: "qrc:/res/image/configuration.png"
			}
		}

		// general icon
		Rectangle {
			id: general
			width: 50
			height: 50
			z: 3
			anchors.horizontalCenter: parent.horizontalCenter
			anchors.top: configure.bottom
			anchors.topMargin: 30
			color: "transparent"

			Image {
				id: generalImage
				width: parent.width
				height: parent.height
				fillMode: Image.Stretch
				smooth: true
				source: "qrc:/res/image/general.png"
			}
		}

		// info icon
		Rectangle {
			id: info
			width: 50
			height: 50
			z: 3
			anchors.horizontalCenter: parent.horizontalCenter
			anchors.top: general.bottom
			anchors.topMargin: 30
			color: "transparent"

			Image {
				id: infoImage
				width: parent.width
				height: parent.height
				fillMode: Image.Stretch
				smooth: true
				source: "qrc:/res/image/info.png"
			}
		}
	}
}
