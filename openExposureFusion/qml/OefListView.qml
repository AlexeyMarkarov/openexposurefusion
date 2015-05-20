import QtQuick 2.4
import QtQuick.Controls 1.2
import QtQuick.Layouts 1.1
import QtQuick.Controls.Styles 1.2
import QtGraphicalEffects 1.0
import ImageElement 1.0
import QmlHelper 1.0

ListView {
    id: mainElem
    width: 75
    height: 75
    snapMode: ListView.SnapToItem
    orientation: ListView.Horizontal

    QmlHelper { id: helper }
    SystemPalette { id: syspal }

    property Action actionAdd
    property Action actionRemove

    delegate: Rectangle {
        id: delegateRect
        height: mainElem.height
        width: height
        color: "transparent"

        ImageElement {
            id: elemImg
            anchors {
                bottom: elemTxtRect.top
                horizontalCenter: parent.horizontalCenter
                leftMargin: 5
                rightMargin: 5
                bottomMargin: 5
            }

            Binding on img {
                when: model.thumbnail !== undefined
                value: model.thumbnail
            }

            Behavior on width { NumberAnimation {duration: helper.doubleClickInterval() / 2} }
            Behavior on height { NumberAnimation {duration: helper.doubleClickInterval() / 3} }
        }

        Rectangle {
            id: elemTxtRect
            anchors {
                bottom: parent.bottom
            }
            height: elemTxt.implicitHeight + elemTxt.anchors.margins * 2
            radius: 3

            Text {
                id: elemTxt
                elide: Qt.ElideLeft
                horizontalAlignment: Qt.AlignCenter
                verticalAlignment: Qt.AlignCenter
                anchors {
                    fill: parent
                    margins: 5
                }
                renderType: Text.NativeRendering

                Binding on text {
                    when: model.filepath !== undefined
                    value: model.filepath
                }
            }
        }

        MouseArea {
            id: mouse
            anchors.fill: parent
            hoverEnabled: true
            onEntered: {
                delegateRect.state = "hovered"
            }
            onExited: {
                delegateRect.state = "normal"
            }
            onPositionChanged: {
                delegateRect.state = pressed ? "normal" : "hovered"
            }
            onReleased: {
                delegateRect.state = "hovered"
            }
            onClicked: {
                mainElem.currentIndex = mainElem.currentIndex == index ? -1 : index
            }
        }

        states: [
            State {
                name: "normal"
                PropertyChanges {
                    target: elemImg
                    width: delegateRect.width - anchors.leftMargin - anchors.rightMargin
                    height: delegateRect.height - elemTxtRect.height - anchors.bottomMargin
                }
                PropertyChanges {
                    target: delegateRect
                    z: 0
                }
                AnchorChanges {
                    target: elemTxtRect
                    anchors.left: delegateRect.left
                    anchors.right: delegateRect.right
                    anchors.horizontalCenter: undefined
                }
                PropertyChanges {
                    target: elemTxtRect
                    width: delegateRect.width
                    color: "transparent"
                }
                PropertyChanges {
                    target: elemTxt
                    color: syspal.text
                }
            },
            State {
                name: "hovered"
                PropertyChanges {
                    target: elemImg
                    width: delegateRect.width * 2
                    height: delegateRect.height * 2
                }
                PropertyChanges {
                    target: delegateRect
                    z: 1
                }
                AnchorChanges {
                    target: elemTxtRect
                    anchors.left: undefined
                    anchors.right: undefined
                    anchors.horizontalCenter: delegateRect.horizontalCenter
                }
                PropertyChanges {
                    target: elemTxtRect
                    width: elemTxt.implicitWidth + elemTxt.anchors.margins * 2
                    color: syspal.highlight
                }
                PropertyChanges {
                    target: elemTxt
                    color: syspal.highlightedText
                }
            }
        ]
        state: "normal"
    }

    highlight: Rectangle {
        color: syspal.window
        z: -1
        radius: 5

        LinearGradient {
            anchors.fill: parent
            source: parent

            gradient: Gradient {
                GradientStop { position: 0.0; color: "transparent" }
                GradientStop { position: 0.5; color: "transparent" }
                GradientStop {
                    position: 1.0;
                    color: Qt.tint(syspal.window, Qt.rgba(syspal.highlight.r,
                                                          syspal.highlight.g,
                                                          syspal.highlight.b,
                                                          0.5))}
            }
        }
    }
    highlightMoveDuration: helper.doubleClickInterval() / 2

    headerPositioning: ListView.OverlayHeader
    header: Rectangle {
        id: headerRect
        height: mainElem.height
        width: headerLayout.implicitWidth
        color: "transparent"
        z: 2

        ColumnLayout {
            id: headerLayout
            anchors {
                centerIn: parent
            }
            Button {
                action: mainElem.actionAdd
                style: OefButtonStyle {}
            }
            Button {
                action: mainElem.actionRemove
                style: OefButtonStyle {}
            }
        }
    }

    add: Transition { NumberAnimation {
            properties: "scale"
            from: 0.0
            to: 1.0
            duration: helper.doubleClickInterval() / 2
        }
    }
    remove: Transition { ParallelAnimation {
            NumberAnimation {
                properties: "scale"
                to: 0.0
                duration: helper.doubleClickInterval() / 2
            }
            NumberAnimation {
                properties: "y"
                to: mainElem.height
                duration: helper.doubleClickInterval() / 2
            }
        }
    }
    displaced: Transition { NumberAnimation { properties: "x,y"; duration: helper.doubleClickInterval() / 2 } }

    Rectangle {
        id: dropTextRect
        height: 20
        width: 20
        anchors {
            fill: parent
            margins: 40
            topMargin: 0
            bottomMargin: 0
        }
        color: "transparent"
        opacity: (typeof mainElem.model !== "undefined") ? mainElem.model.empty : 1

        Behavior on opacity {NumberAnimation {duration: helper.doubleClickInterval() / 2}}

        Rectangle {
            anchors.fill: parent
            color: "transparent"

            RadialGradient {
                anchors.fill: parent
                gradient: Gradient {
                    GradientStop { position: 0.0; color: syspal.highlight }
                    GradientStop { position: 0.5; color: "transparent" }
                }
            }
        }

        Text {
            text: qsTr("Drop the pictures here")
            horizontalAlignment: Qt.AlignHCenter
            verticalAlignment: Qt.AlignVCenter
            anchors {
                fill: parent
                leftMargin: mainElem.header.width
            }
            fontSizeMode: Text.Fit
            font.pixelSize: Math.min(parent.width, parent.height) / 3
            color: syspal.highlightedText
            renderType: Text.NativeRendering
        }
    }

    DropArea {
        anchors.fill: parent
        onDropped: {
            if(drop.hasUrls){
                if(mainElem.model !== undefined)
                    mainElem.model.add(drop.urls)
            }
        }
    }
}

