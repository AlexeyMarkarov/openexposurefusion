import QtQuick 2.2
import QtQuick.Controls.Styles 1.2
import QtQuick.Controls 1.2
import QtGraphicalEffects 1.0

ProgressBarStyle {
    SystemPalette {id: syspal}

    progress: Rectangle {
        color: "transparent"

        Rectangle {
            radius: 3
            anchors {
                fill: parent
                margins: 2
            }
            border {
                color: syspal.shadow
                width: 1
            }

            LinearGradient {
                anchors.fill: parent
                start: Qt.point(0, 0)
                end: Qt.point(width, 0)
                source: parent
                gradient: Gradient {
                    GradientStop {position: 0.0; color: Qt.rgba(0.0, 1.0, 0.0, 0.75)}
                    GradientStop {position: 1.0; color: Qt.rgba(currentProgress, 1 - currentProgress, 0.0, 0.75)}
                }
            }
        }
    }
}

