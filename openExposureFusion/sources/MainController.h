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

#ifndef MAINCONTROLLER_H
#define MAINCONTROLLER_H

#include <QtCore>
#include "models/FilesModel.h"
#include "models/DevicesModel.h"
#include "models/DeviceInfoModel.h"
#include "gui/MainWindow.h"
#include "wrappersCL/ClPlatform.h"
#include "MertensCl.h"

class MainController : public QObject
{
    Q_OBJECT

public:
    MainController(QObject *parent = 0);
    ~MainController();

    bool init();
    void release();

protected:
    virtual void timerEvent(QTimerEvent *event);

private slots:
    void onPropertyChanged(const MainWindow::PropertyType type);
    void onFinished(const QImage result);
    void onInputListChanged();
    void onSaveClicked();
    void onUpdateViewClicked();

private:
    static void debugMessageHandler(QtMsgType type, const QMessageLogContext &context, const QString &msg);
    static MainController *sCtrl;
    static QMutex sDebugMutex;
    static QStringList sDebugQueue;

    MainWindow *mWnd;
    QStringList mFileFormatsModel;
    FilesModel mInputFilesModel;
    DevicesModel mDevicesModel;
    MertensCl mExpoFusion;
    DeviceInfoModel mDeviceInfoModel;
    bool mScheduleUpdate;
    QTime mProcessingTime;
    int mProcessingTimerId;
    bool mIsProcessing;

    QThread mThreadForCore;

    void initQt();
    void initUi();
    void initCore();
    void initOpenCLDevices();
    bool initFusion();
    void connectSignals();

    void loadSettings();
    void processImages();
    void updateDeviceWarning();
    void updateMemoryUsage();
};

#endif // MAINCONTROLLER_H
