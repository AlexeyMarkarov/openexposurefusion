import QtQuick 2.0
import QtQuick.Controls 1.2
import QtQuick.Layouts 1.1
import QtQuick.Controls.Styles 1.2

Rectangle {
    implicitHeight: layout.implicitHeight
    implicitWidth: layout.implicitWidth
    color: "transparent"

    property alias text: txt.text
    property alias value: spin.value

    RowLayout {
        id: layout
        anchors.fill: parent

        Text {
            id: txt
            renderType: Text.NativeRendering
        }
        Item { Layout.fillWidth: true }
        SpinBox {
            id: spin
            minimumValue: 0
            maximumValue: 1
            stepSize: 0.01
            decimals: 2
            style: SpinBoxStyle{}
            Layout.minimumWidth: 60
            onValueChanged: slider.value = value
        }
        Slider {
            id: slider
            style: SliderStyle{}
            onValueChanged: spin.value = value
        }
    }
}

