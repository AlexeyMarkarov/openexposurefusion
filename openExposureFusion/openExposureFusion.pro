#-------------------------------------------------
#
# Project created by QtCreator 2015-01-07T00:44:54
#
#-------------------------------------------------

QT       += core gui opengl qml quick concurrent

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

CONFIG += c++11
QMAKE_CXXFLAGS += -Wall

BUILDCONF = unknown
CONFIG(debug, debug|release):BUILDCONF = debug
CONFIG(release, debug|release):BUILDCONF = release

TARGET = openExposureFusion_$$QT_ARCH
TEMPLATE = app

DESTDIR = ./$$BUILDCONF/$$QT_ARCH/bin
MOC_DIR = ./$$BUILDCONF/$$QT_ARCH/moc
OBJECTS_DIR = ./$$BUILDCONF/$$QT_ARCH/obj
RCC_DIR = ./$$BUILDCONF/$$QT_ARCH/rcc
UI_DIR = ./$$BUILDCONF/$$QT_ARCH/ui

INCLUDEPATH += ./sources \
    ./dependencies \
DEPENDPATH += ./sources \
    ./dependencies
VPATH += ./sources \
    ./dependencies

SOURCES += main.cpp \
    MainController.cpp \
    FileInfo.cpp \
    Settings.cpp \
    Util.cpp \
    MertensCl.cpp \
    gui/MainWindow.cpp \
    gui/ImageElement.cpp \
    gui/QmlPixmapProvider.cpp \
    gui/QmlHelper.cpp \
    models/FilesModel.cpp \
    models/DevicesModel.cpp \
    models/DeviceInfoModel.cpp \
    clew/clew.c \
    wrappersCL/ClPlatform.cpp \
    wrappersCL/ClDevice.cpp \
    wrappersCL/ClProgram.cpp

HEADERS  += \
    MainController.h \
    FileInfo.h \
    Settings.h \
    Util.h \
    MertensCl.h \
    gui/MainWindow.h \
    gui/ImageElement.h \
    gui/QmlPixmapProvider.h \
    gui/QmlHelper.h \
    models/FilesModel.h \
    models/DevicesModel.h \
    models/DeviceInfoModel.h \
    clew/clew.h \
    wrappersCL/ClPlatform.h \
    wrappersCL/ClDevice.h \
    wrappersCL/ClProgram.h

RESOURCES += \
    resources.qrc

OTHER_FILES += \
    qml/MainWindow.qml \
    qml/MeasureControl.qml \
    qml/AboutDialog.qml \
    qml/DebugLogDialog.qml \
    qml/OefButtonStyle.qml \
    qml/OefMenuBarStyle.qml \
    qml/OefListView.qml \
    qml/OefProgressBarStyle.qml \
    resources/mertens.cl \
    resources/gpl3.txt \
    resources/exposureFusion.txt
