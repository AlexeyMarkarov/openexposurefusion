import QtQuick 2.3
import QtQuick.Controls 1.2
import QtQuick.Controls.Styles 1.2
import QtQuick.Layouts 1.1
import QtQuick.Window 2.0

Window {
    id: mainElem
    color: syspal.window
    flags: Qt.Dialog
           | Qt.CustomizeWindowHint
           | Qt.WindowTitleHint
           | Qt.WindowSystemMenuHint
           | Qt.WindowCloseButtonHint
    minimumWidth: 500
    minimumHeight: 300

    SystemPalette {id: syspal}

    ColumnLayout {
        id: mainlayout
        anchors {
            fill: parent
            margins: 10
        }

        TextArea {
            id: textArea
            style: TextAreaStyle {}
            readOnly: true
            Layout.fillWidth: true
            Layout.fillHeight: true
        }

        RowLayout {
            Item {Layout.fillWidth: true}
            Button {
                text: qsTr("OK")
                style: OefButtonStyle {}
                onClicked: mainElem.close()
            }
            Item {Layout.fillWidth: true}
        }
    }

    function append(str){
        textArea.append(str)
    }
}
