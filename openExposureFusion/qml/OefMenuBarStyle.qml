import QtQuick 2.0
import QtQuick.Controls 1.2
import QtQuick.Controls.Styles 1.2
import QtQuick.Layouts 1.1
import QmlHelper 1.0

MenuBarStyle {
    SystemPalette {id: syspal}
    QmlHelper {id: helper}

    readonly property color topColor: Qt.tint(syspal.window, Qt.rgba(syspal.shadow.r, syspal.shadow.g, syspal.shadow.b, 0.5))
    readonly property color bottomColor: syspal.window

    background: Rectangle {
        height: 100
        gradient: Gradient {
            GradientStop {
                position: 0
                color: topColor
            }
            GradientStop {
                position: 1
                color: bottomColor
            }
        }
    }

    itemDelegate: Rectangle {
        implicitHeight: itemTxt.implicitHeight * 1.5
        implicitWidth: itemTxt.implicitWidth * 2
        gradient: Gradient {
            GradientStop {
                position: 0
                color: styleData.open
                       ? Qt.tint(syspal.window, Qt.rgba(syspal.highlight.r, syspal.highlight.g, syspal.highlight.b, 0.5))
                       : topColor
                Behavior on color { ColorAnimation {duration: helper.doubleClickInterval() / 4} }
            }
            GradientStop {
                position: 1
                color: bottomColor
            }
        }
        border {
            color: syspal.shadow
            width: styleData.selected || styleData.open
        }
        Text {
            id: itemTxt
            anchors.centerIn: parent
            text: formatMnemonic(styleData.text, styleData.underlineMnemonic)
            font.pointSize: 10
            renderType: Text.NativeRendering
        }
    }

    menuStyle: MenuStyle {
        frame: Rectangle {
            color: syspal.alternateBase
        }
        separator: Rectangle {
            implicitHeight: 1
            implicitWidth: 1
            color: syspal.shadow
        }
        itemDelegate.background: Rectangle {
            border {
                width: styleData.selected
                color: syspal.shadow
            }
            gradient: Gradient {
                GradientStop {
                    position: 0;
                    color: "transparent"
                }
                GradientStop {
                    position: 1;
                    color: styleData.enabled && styleData.selected
                           ? Qt.rgba(syspal.highlight.r, syspal.highlight.g, syspal.highlight.b, 0.5)
                           : "transparent"
                    Behavior on color { ColorAnimation { duration: helper.doubleClickInterval() / 4 } }
                }
            }
            radius: 3
        }
        itemDelegate.label: Text {
            text: formatMnemonic(styleData.text, styleData.underlineMnemonic)
            renderType: Text.NativeRendering
            font.pointSize: 10
        }
    }
}

