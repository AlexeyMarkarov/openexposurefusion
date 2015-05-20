import QtQuick 2.0
import QtQuick.Controls 1.2
import QtQuick.Controls.Styles 1.2
import QtQuick.Layouts 1.1
import QmlHelper 1.0

ButtonStyle {
    SystemPalette {id: syspal}
    QmlHelper {id: helper}

    background: Rectangle {
        readonly property color pressedColor: Qt.tint(syspal.button, Qt.rgba(syspal.highlight.r, syspal.highlight.g, syspal.highlight.b, 0.75))
        readonly property color hoveredColor: Qt.tint(syspal.button, Qt.rgba(syspal.highlight.r, syspal.highlight.g, syspal.highlight.b, 0.25))
        readonly property color normalColor: Qt.tint(syspal.button, Qt.rgba(syspal.highlight.r, syspal.highlight.g, syspal.highlight.b, 0.1))
        border {
            color: control.enabled ? syspal.shadow : syspal.mid
            width: 1
        }
        radius: control.hovered ? 9 : 3
        color: control.pressed
               ? pressedColor
               : control.hovered
                 ? hoveredColor
                 : normalColor
        Behavior on color { ColorAnimation {duration: helper.doubleClickInterval() / 4} }
        Behavior on radius { NumberAnimation {duration: helper.doubleClickInterval() / 4} }
    }

    label: Item {
        implicitWidth: Math.max(textLayout.implicitWidth, 30)
        implicitHeight: Math.max(textLayout.implicitHeight, 30)
        RowLayout {
            id: textLayout
            spacing: 3
            anchors.centerIn: parent
            Image {
                source: control.iconSource === undefined ? control.iconName : control.iconSource
            }
            Text {
                text: control.text
                color: !control.enabled
                       ? syspal.shadow
                       : control.pressed
                         ? syspal.highlightedText
                         : syspal.buttonText
                renderType: Text.NativeRendering
                Behavior on color { ColorAnimation {duration: helper.doubleClickInterval() / 4} }
            }
        }
    }
}

