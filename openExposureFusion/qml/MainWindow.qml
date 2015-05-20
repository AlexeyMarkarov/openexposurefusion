import QtQuick 2.4
import QtQuick.Window 2.0
import QtQuick.Controls 1.2
import QtQuick.Layouts 1.1
import QtQuick.Dialogs 1.2
import QtQuick.Controls.Styles 1.2
import ImageElement 1.0
import QmlHelper 1.0

ApplicationWindow {
    id: window
    visible: false
    color: syspal.window
    minimumHeight: mainLayout.implicitHeight + mainLayout.anchors.margins * 2 + statusBar.height * 2
    minimumWidth: mainLayout.implicitWidth * 2 + mainLayout.anchors.margins * 2
    width: minimumWidth
    height: minimumHeight

    SystemPalette { id: syspal }
    QmlHelper {id: helper }
    FileDialog {
        id: dlgOpenFile
        selectExisting: true
        selectFolder: false
        selectMultiple: true
        title: qsTr("Select files...")

        onAccepted: {
            filesModel.add(fileUrls)
        }
    }
    FileDialog {
        id: dlgSelectDir
        selectExisting: true
        selectFolder: true
        selectMultiple: false
        title: qsTr("Select directory...")

        onAccepted: {
            outputDir.text = helper.toLocalFile(fileUrl)
        }
    }
    AboutDialog {
        id: dlgAbout
        title: qsTr("About")
    }
    DebugLogDialog {
        id: dlgDebug
        title: qsTr("Debug log")
    }

    Item {
        Action {
            id: actionAdd
            tooltip: qsTr("Add images")
            onTriggered: dlgOpenFile.open()
            iconSource: "image://provider/stdicons/SP_FileDialogStart"
        }
        Action {
            id: actionRemove
            tooltip: qsTr("Remove selected")
            iconSource: "image://provider/stdicons/SP_TrashIcon"
            onTriggered: {
                if(table.currentIndex >= 0) filesModel.remove(table.currentIndex)
                if(filesModel.empty) table.currentIndex = -1
            }
        }
        Action {
            id: actionExit
            tooltip: qsTr("Exit")
            iconSource: "image://provider/stdicons/SP_TitleBarCloseButton"
            onTriggered: close()
        }
        Action {
            id: actionDebugLog
            tooltip: qsTr("Debug log")
            onTriggered: {
                dlgDebug.show()
            }
        }
        Action {
            id: actionAboutApp
            tooltip: qsTr("About app")
            onTriggered: {
                dlgAbout.show()
            }
        }
        Action {
            id: actionAboutQt
            tooltip: qsTr("About Qt")
            onTriggered: helper.aboutQt()
        }
        Action {
            id: actionSave
            tooltip: qsTr("Save result")
            iconSource: "image://provider/stdicons/SP_DialogSaveButton"
        }
        Action {
            id: actionBrowseOutDir
            tooltip: qsTr("Select directory")
            onTriggered: dlgSelectDir.open()
            iconSource: "image://provider/stdicons/SP_DirIcon"
        }
        Action {
            id: actionUpdateView
            tooltip: qsTr("Update result")
            iconSource: "image://provider/stdicons/SP_BrowserReload"
        }
    }

    property alias filesModel:              table.model
    property alias resultImg:               imgResult.img
    property alias formatsModel:            cboxOutput.model
    property alias formatsIndex:            cboxOutput.currentIndex
    property alias autoUpdate:              chkAutoUpdateView.checked
    property alias measureContrastValue:    measureContrast.value
    property alias measureSaturationValue:  measureSaturation.value
    property alias measureExposednessValue: measureExposedness.value
    property alias outputDir:               outputDir.text
    property alias progress:                progressBar.opacity
    property alias devicesModel:            cboxDevices.model
    property alias devicesIndex:            cboxDevices.currentIndex
    property alias devicesPropertyModel:    tableDevices.model
    property alias deviceWarningVisible:    deviceWarning.visible
    property alias statusText:              textStatus.text
    property alias memoryText:              textMemory.text
    property alias memoryProgress:          progressMemory.value

    onFilesModelChanged: {}
    onResultImgChanged: {}
    onFormatsModelChanged: {}
    onFormatsIndexChanged: {}
    onAutoUpdateChanged: {}
    onMeasureContrastValueChanged: {}
    onMeasureSaturationValueChanged:  {}
    onMeasureExposednessValueChanged: {}
    onOutputDirChanged: {}
    onProgressChanged: {}
    onDevicesModelChanged: {}
    onDevicesIndexChanged: {}
    onDevicesPropertyModelChanged: {}
    onDeviceWarningVisibleChanged: {}

    signal saveClicked
    signal updateViewClicked

    function resizeDevicesTable() {
        tableDevices.resizeColumnsToContents()
    }

    statusBar: StatusBar {
        id: status
        style: StatusBarStyle{}

        RowLayout {
            anchors.fill: parent

            ProgressBar {
                id: progressBar
                style: ProgressBarStyle{}
                opacity: 0
                indeterminate: true
                Layout.fillWidth: true
                Behavior on opacity { NumberAnimation {duration: helper.doubleClickInterval() / 4} }
            }
            Text {
                id: textStatus
                renderType: Text.NativeRendering
            }
        }
    }

    menuBar: MenuBar {
        style: OefMenuBarStyle{}
        Menu {
            title: qsTr("&File")
            MenuItem {
                text: qsTr("&Add...")
                action: actionAdd
            }
            MenuItem {
                text: qsTr("&Remove")
                action: actionRemove
            }
            MenuItem {
                text: qsTr("&Select output dir...")
                action: actionBrowseOutDir
            }
            MenuSeparator {}
            MenuItem {
                text: qsTr("&Exit")
                action: actionExit
            }
        }
        Menu {
            title: qsTr("&Help")
            MenuItem {
                text: qsTr("&Debug log...")
                action: actionDebugLog
            }
            MenuItem {
                text: qsTr("A&bout ") + Qt.application.name + "..."
                action: actionAboutApp
            }
            MenuItem {
                text: qsTr("About &Qt...")
                action: actionAboutQt
            }
        }
    }

    ColumnLayout {
        id: mainLayout
        anchors.fill: parent
        anchors.margins: 10

        RowLayout {
            Rectangle {
                color: "transparent"
                Layout.fillHeight: true
                Layout.fillWidth: true

                GroupBox {
                    anchors.fill: parent
                    title: qsTr("Output")

                    ColumnLayout {
                        anchors.fill: parent
                        ImageElement {
                            id: imgResult
                            Layout.fillHeight: true
                            Layout.fillWidth: true
                        }
                    }
                }
            }

            Rectangle {
                color: syspal.window
                Layout.minimumWidth: optionsLayout.implicitWidth
                Layout.minimumHeight: optionsLayout.implicitHeight
                Layout.fillHeight: true

                ColumnLayout {
                    id: optionsLayout
                    anchors.fill: parent

                    GroupBox {
                        title: qsTr("View")
                        Layout.fillWidth: true

                        RowLayout {
                            anchors.fill: parent

                            Button {
                                text: qsTr("Update")
                                action: actionUpdateView
                                style: OefButtonStyle {}
                            }
                            CheckBox {
                                id: chkAutoUpdateView
                                text: qsTr("Auto")
                                style: CheckBoxStyle{}
                            }
                            Item { Layout.fillWidth: true }
                        }
                    }

                    GroupBox {
                        title: qsTr("Device")
                        Layout.fillWidth: true
                        Layout.fillHeight: true

                        ColumnLayout {
                            anchors.fill: parent

                            RowLayout {
                                ComboBox {
                                    id: cboxDevices
                                    style: ComboBoxStyle {}
                                    Layout.fillWidth: true
                                }
                                Image {
                                    id: deviceWarning
                                    source: "image://provider/stdicons/SP_MessageBoxWarning"
                                    visible: false
                                    sourceSize.height: cboxDevices.height
                                    sourceSize.width: cboxDevices.height

                                    Rectangle {
                                        anchors {
                                            bottom: parent.top
                                            right: parent.right
                                        }
                                        width: deviceWarningText.paintedWidth + 10
                                        height: deviceWarningText.paintedHeight + 10
                                        color: syspal.window
                                        border {
                                            width: 1
                                            color: syspal.shadow
                                        }
                                        radius: 3
                                        visible: mouseDeviceWarning.containsMouse

                                        Text {
                                            id: deviceWarningText
                                            anchors.centerIn: parent
                                            text: qsTr("Select a GPU device for best performance")
                                            renderType: Text.NativeRendering
                                        }
                                    }

                                    MouseArea {
                                        id: mouseDeviceWarning
                                        anchors.fill: parent
                                        hoverEnabled: true
                                    }
                                }
                            }

                            TableView {
                                id: tableDevices
                                style: TableViewStyle {}
                                selectionMode: SelectionMode.NoSelection
                                Layout.fillWidth: true
                                Layout.fillHeight: true

                                TableViewColumn { role: "propertyname" }
                                TableViewColumn { role: "propertyvalue" }
                            }
                        }
                    }

                    GroupBox {
                        title: qsTr("Measures")
                        Layout.fillWidth: true

                        ColumnLayout {
                            anchors.fill: parent

                            MeasureControl {
                                id: measureContrast
                                text: qsTr("Contrast")
                                Layout.fillWidth: true
                            }
                            MeasureControl {
                                id: measureSaturation
                                text: qsTr("Saturation")
                                Layout.fillWidth: true
                            }
                            MeasureControl {
                                id: measureExposedness
                                text: qsTr("Exposedness")
                                Layout.fillWidth: true
                            }
                        }
                    }

                    GroupBox {
                        title: qsTr("Output file")
                        Layout.fillWidth: true

                        GridLayout {
                            anchors.fill: parent
                            columns: 3
                            rows: 3

                            Text {
                                text: qsTr("Format")
                                renderType: Text.NativeRendering
                                Layout.column: 0
                                Layout.row: 0
                            }
                            ComboBox {
                                id: cboxOutput
                                style: ComboBoxStyle{}
                                Layout.column: 1
                                Layout.row: 0
                                Layout.columnSpan: 2
                                Layout.fillWidth: true
                            }
                            Text {
                                text: qsTr("Directory")
                                renderType: Text.NativeRendering
                                Layout.column: 0
                                Layout.row: 1
                            }
                            TextField {
                                id: outputDir
                                style: TextFieldStyle{}
                                Layout.column: 1
                                Layout.row: 1
                                Layout.fillWidth: true
                            }
                            Button {
                                text: qsTr("Browse...")
                                action: actionBrowseOutDir
                                style: OefButtonStyle {}
                                Layout.column: 2
                                Layout.row: 1
                            }
                            Button {
                                text: qsTr("Save")
                                action: actionSave
                                style: OefButtonStyle {}
                                Layout.column: 0
                                Layout.columnSpan: 3
                                Layout.row: 2
                                Layout.fillWidth: true
                            }
                        }
                    }

                    GroupBox {
                        title: qsTr("Device memory usage")
                        Layout.fillWidth: true

                        ColumnLayout {
                            anchors.fill: parent

                            ProgressBar {
                                id: progressMemory
                                style: OefProgressBarStyle{}
                                minimumValue: 0
                                maximumValue: 100
                                Layout.fillWidth: true
                            }
                            Text {
                                id: textMemory
                                renderType: Text.NativeRendering
                                Layout.alignment: Layout.Center
                            }
                        }
                    }
                }
            }
        }

        Rectangle {
            height: helper.desktopSize().height / 10
            Layout.fillWidth: true
            color: "transparent"

            RowLayout {
                anchors.fill: parent

                OefListView {
                    id: table
                    actionAdd: actionAdd
                    actionRemove: actionRemove
                    Layout.fillWidth: true
                    Layout.fillHeight: true
                }
            }
        }
    }

    Component.onCompleted: {
        actionSave.triggered.connect(saveClicked)
        actionUpdateView.triggered.connect(updateViewClicked)
    }

    function appendDebugStr(str){
        dlgDebug.append(str)
    }
}
