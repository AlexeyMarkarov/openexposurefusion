import QtQuick 2.3
import QtQuick.Controls 1.2
import QtQuick.Controls.Styles 1.2
import QtQuick.Layouts 1.1
import QtQuick.Window 2.0
import QmlHelper 1.0

Window {
    id: mainElem
    color: syspal.window
    modality: Qt.ApplicationModal
    flags: Qt.Dialog
           | Qt.CustomizeWindowHint
           | Qt.WindowTitleHint
           | Qt.WindowSystemMenuHint
           | Qt.WindowCloseButtonHint
    minimumWidth: 500
    minimumHeight: 300

    SystemPalette {id: syspal}
    QmlHelper {id: helper}

    ColumnLayout {
        id: mainlayout
        anchors {
            fill: parent
            margins: 10
        }

        TabView {
            style: TabViewStyle {}
            Layout.fillWidth: true
            Layout.fillHeight: true

            Tab {
                title: qsTr("About")

                Rectangle {
                    anchors.fill: parent
                    color: "transparent"

                    Text {
                        anchors {
                            top: parent.top
                            horizontalCenter: parent.horizontalCenter
                            margins: 10
                        }
                        renderType: Text.NativeRendering
                        textFormat: Text.RichText

                        text: qsTr("<p align=center>") + Qt.application.name + "</p>"
                              + qsTr("HDR images generation from bracketed LDR images.<br/>")
                              + qsTr("Implements the Exposure Fusion algorithm by Mertens-Kautz-Van Reeth.<br/><br/>")
                              + qsTr("Version: ") + Qt.application.version + "<br/>"
                              + qsTr("Homepage: ") + "<a href=\"http://sourceforge.net/projects/openexposurefusion\">"
                              + "http://sourceforge.net/projects/openexposurefusion</a><br/>"
                              + qsTr("Contact: ")
                              + "<a href=\"openexposurefusion@gmail.com\">openexposurefusion@gmail.com</a><br/>"
                              + qsTr("Author: Alexey Markarov") + "<br/><br/>"
                              + qsTr("Created with:")
                              + "<ul><li>"
                              + "Qt <a href=\"http://www.qt.io\">http://www.qt.io</a>"
                              + "</li><li>"
                              + "OpenCL <a href=\"http://www.khronos.org/opencl\">http://www.khronos.org/opencl</a>"
                              + "</li></ul>"

                        onLinkActivated: Qt.openUrlExternally(link)
                    }
                }
            }
            Tab {
                title: qsTr("License")

                TextArea {
                    anchors.margins: 10
                    style: TextAreaStyle {}
                    readOnly: true
                    text: helper.readFile(":/license/gpl3.txt")
                          + "\n\n////////////////////////////////////////////////////////////////////////////////////////////////////\n\n"
                          + helper.readFile(":/license/exposureFusion.txt")
                }
            }
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
}
