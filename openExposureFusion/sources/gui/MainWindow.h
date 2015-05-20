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

#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QtCore>
#include <QtGui>
#include <QtWidgets>
#include <QtQml>

class MainWindow : public QObject
{
    Q_OBJECT

public:
    enum PropertyType
    {
        PT_InputFilesModel = 0,
        PT_AutoUpdate,
        PT_MeasureContrast,
        PT_MeasureSaturation,
        PT_MeasureExposedness,
        PT_OutputFormatModel,
        PT_OutputFormatIndex,
        PT_OutputDir,
        PT_ResultImage,
        PT_Progress,
        PT_DevicesModel,
        PT_DevicesIndex,
        PT_DevicesPropertyModel,
        PT_Title,
        PT_DeviceWarningVisible,
        PT_StatusText,
        PT_MemoryText,
        PT_MemoryProgress,
        PT_max
    };

    MainWindow(QObject *parent = 0);
    ~MainWindow();

    QStringList getCreationErrors()const;
    QVariant getProperty(const PropertyType type)const;

public slots:
    void setProperty(const PropertyType type, const QVariant value);
    void setVisible(const bool visible);
    void resizeDevicesTable();
    void appendDebugStr(const QString str);

signals:
    void propertyChanged(const MainWindow::PropertyType type);
    void saveClicked();
    void updateViewClicked();

private slots:
    void onMapped(int value);

private:
    QQmlEngine mEngine;
    QQmlComponent *mRootComponent;
    QObject *mWindow;

    void createMembers();
    void setupMembers();
    void connectSignals();
};

#endif // MAINWINDOW_H
