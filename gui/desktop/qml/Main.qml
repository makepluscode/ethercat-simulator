import QtQuick 2.15
import QtQuick.Controls 2.15

ApplicationWindow {
    visible: true
    width: 480
    height: 320
    title: qsTr("EtherCAT Simulator GUI")

    Column {
        anchors.centerIn: parent
        spacing: 12

        Label { text: qsTr("Hello EtherCAT (Qt6/QML)") }
        Button {
            text: qsTr("Close")
            onClicked: Qt.quit()
        }
    }
}

