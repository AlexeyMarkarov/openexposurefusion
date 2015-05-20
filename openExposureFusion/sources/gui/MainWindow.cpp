/*
openExposureFusion
Copyright (C) 2015 Alexey Markarov

This program is free software: you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation, either version 3 of the License, or
(at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/

#include "MainWindow.h"
#include "QmlPixmapProvider.h"

const QMap<MainWindow::PropertyType /*type*/, QString/*qml name*/> kProperties = {
    {MainWindow::PT_InputFilesModel,        "filesModel"},
    {MainWindow::PT_AutoUpdate,             "autoUpdate"},
    {MainWindow::PT_MeasureContrast,        "measureContrastValue"},
    {MainWindow::PT_MeasureSaturation,      "measureSaturationValue"},
    {MainWindow::PT_MeasureExposedness,     "measureExposednessValue"},
    {MainWindow::PT_OutputFormatModel,      "formatsModel"},
    {MainWindow::PT_OutputFormatIndex,      "formatsIndex"},
    {MainWindow::PT_OutputDir,              "outputDir"},
    {MainWindow::PT_ResultImage,            "resultImg"},
    {MainWindow::PT_Progress,               "progress"},
    {MainWindow::PT_DevicesModel,           "devicesModel"},
    {MainWindow::PT_DevicesIndex,           "devicesIndex"},
    {MainWindow::PT_DevicesPropertyModel,   "devicesPropertyModel"},
    {MainWindow::PT_Title,                  "title"},
    {MainWindow::PT_DeviceWarningVisible,   "deviceWarningVisible"},
    {MainWindow::PT_StatusText,             "statusText"},
    {MainWindow::PT_MemoryText,             "memoryText"},
    {MainWindow::PT_MemoryProgress,         "memoryProgress"}
};

MainWindow::MainWindow(QObject *parent)
    : QObject(parent)
{
    createMembers();
    setupMembers();
    connectSignals();
}

MainWindow::~MainWindow()
{
}

void MainWindow::createMembers()
{
    mRootComponent = new QQmlComponent(&mEngine);
}

void MainWindow::setupMembers()
{
    mEngine.addImageProvider("provider", new QmlPixmapProvider);
    mRootComponent->loadUrl(QUrl("qrc:/qml/MainWindow.qml"));
    if(QObject *obj = mRootComponent->create())
    {
        mWindow = obj;
        mWindow->setParent(this);
    }
    else
    {
        qDebug() << mRootComponent->errors();
        mWindow = new QObject(this);
    }
}

void MainWindow::connectSignals()
{
    for(auto iter = kProperties.constBegin(); iter != kProperties.constEnd(); ++iter)
    {
        QSignalMapper *mapper = new QSignalMapper(this);
        mapper->setMapping(mWindow, iter.key());
        connect(mapper, SIGNAL(mapped(int)), SLOT(onMapped(int)));

        const QMetaObject *meta = mapper->metaObject();
        connect(mWindow, QQmlProperty(mWindow, iter.value()).property().notifySignal(),
                mapper, meta->method(meta->indexOfMethod("map()")));
    }
    connect(mWindow, SIGNAL(saveClicked()), SIGNAL(saveClicked()));
    connect(mWindow, SIGNAL(updateViewClicked()), SIGNAL(updateViewClicked()));
}

QStringList MainWindow::getCreationErrors()const
{
    QStringList errors;
    const QList<QQmlError> qmlerr = mRootComponent->errors();
    for(int i = 0; i < qmlerr.count(); ++i)
    {
        errors.append(qmlerr.at(i).toString());
    }
    return errors;
}

QVariant MainWindow::getProperty(const MainWindow::PropertyType type) const
{
    return QQmlProperty::read(mWindow, kProperties.value(type));
}

void MainWindow::setProperty(const MainWindow::PropertyType type, const QVariant value)
{
    QSignalBlocker blocker(mWindow);
    QQmlProperty::write(mWindow, kProperties.value(type), value);
}

void MainWindow::onMapped(int value)
{
    emit propertyChanged(static_cast<PropertyType>(value));
}

void MainWindow::setVisible(const bool visible)
{
    QQmlProperty::write(mWindow, "visible", visible);
}

void MainWindow::resizeDevicesTable()
{
    QMetaObject::invokeMethod(mWindow, "resizeDevicesTable");
}

void MainWindow::appendDebugStr(const QString str)
{
    QMetaObject::invokeMethod(mWindow, "appendDebugStr", Q_ARG(const QVariant, str));
}
