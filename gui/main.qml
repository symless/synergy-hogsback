import QtQuick 2.3
import QtQuick.Controls 1.2

ApplicationWindow {
	id : applicationWindow
	visible: true
	width: 800
	height: 600
	minimumWidth: 800
	minimumHeight: 600
	title: qsTr("Synergy")


	StackView {
		id: stackView
		anchors.fill: parent
		focus: true

		initialItem: {
			[{item : Qt.resolvedUrl("ConfigurationPage.qml")}]
		}
	}
}

