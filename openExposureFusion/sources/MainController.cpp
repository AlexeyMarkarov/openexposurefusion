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

#include "MainController.h"
#include "gui/ImageElement.h"
#include "Settings.h"
#include "gui/QmlHelper.h"
#include "wrappersCL/ClPlatform.h"
#include "MertensCl.h"
#include "Util.h"

const QMap<Settings::Type, MainWindow::PropertyType> kMapOptionToProperty = {
    {Settings::T_AutoUpdateView,        MainWindow::PT_AutoUpdate},
    {Settings::T_MeasureContrast,       MainWindow::PT_MeasureContrast},
    {Settings::T_MeasureSaturation,     MainWindow::PT_MeasureSaturation},
    {Settings::T_MeasureExposedness,    MainWindow::PT_MeasureExposedness},
    {Settings::T_OutputDir,             MainWindow::PT_OutputDir}
};

const QString kTimeFormat("HH:mm:ss.zzz");
const QString kDateTimeFormat("yyyy/MM/dd-HH:mm:ss.zzz");

MainController *MainController::sCtrl = nullptr;
QMutex MainController::sDebugMutex;
QStringList MainController::sDebugQueue;

void MainController::debugMessageHandler(QtMsgType type, const QMessageLogContext &context, const QString &msg)
{
    Q_UNUSED(context)

    QMutexLocker lock(&sDebugMutex);

    static const QMap<QtMsgType, QString> typeMap = {
        {QtDebugMsg,    "Debug"},
        {QtWarningMsg,  "Warning"},
        {QtCriticalMsg, "Critical"},
        {QtFatalMsg,    "Fatal"},
    };

    const QString finalMsg = QDateTime::currentDateTimeUtc().toString(kDateTimeFormat)
                             + " "
                             + typeMap.value(type, "Msg")
                             + ": "
                             + msg;
    fprintf(stderr, "%s\n", finalMsg.toLocal8Bit().data());
    bool flushed = false;
    if(sCtrl)
    {
        if(sCtrl->mWnd)
        {
            for(int i = 0; i < sDebugQueue.count(); ++i)
            {
                sCtrl->mWnd->appendDebugStr(sDebugQueue.at(i));
            }
            sCtrl->mWnd->appendDebugStr(finalMsg);
            flushed = true;
        }
    }

    if(!flushed)
    {
        sDebugQueue.append(finalMsg);
    }
}

MainController::MainController(QObject *parent)
    : QObject(parent)
    , mWnd(nullptr)
    , mScheduleUpdate(false)
    , mProcessingTimerId(-1)
    , mIsProcessing(false)
{
    sCtrl = this;
}

MainController::~MainController()
{
}

bool MainController::init()
{
    initQt();
    initCore();
    initUi();

    const QStringList wndErrors = mWnd->getCreationErrors();
    if(!wndErrors.isEmpty())
    {
        QMessageBox::critical(0, tr("GUI error"), wndErrors.join("\n"), QMessageBox::Ok);
        return false;
    }

    const int clewResult = clewInit(L"OpenCL");
    switch(clewResult)
    {
        case CLEW_SUCCESS:
            break;

        default:
            QMessageBox::critical(0, tr("OpenCL error"), tr("Can't connect to OpenCL"), QMessageBox::Ok);
            return false;
    }

    initOpenCLDevices();

    if(mDevicesModel.rowCount() <= 0)
    {
        QMessageBox::critical(0,
                              tr("OpenCL error"),
                              tr("Can't find supported OpenCL devices.\n"
                                 "A device must be available/enabled, have a compiler and support 2D images."),
                              QMessageBox::Ok);
        return false;
    }

    if(!initFusion())
    {
        QMessageBox::critical(0, tr("Exposure Fusion error"), tr("Can't initalize OpenCL"), QMessageBox::Ok);
        return false;
    }

    mWnd->setProperty(MainWindow::PT_DevicesIndex, -1);
    mWnd->setProperty(MainWindow::PT_DevicesIndex, 0);
    mDeviceInfoModel.setDevice(mDevicesModel.at(0));
    mWnd->resizeDevicesTable();
    updateDeviceWarning();
    updateMemoryUsage();

    loadSettings();
    connectSignals();

    mWnd->setVisible(true);
    return true;
}

void MainController::initQt()
{
    qInstallMessageHandler(debugMessageHandler);

    qApp->setOrganizationName("openExposureFusion");
    qApp->setApplicationName("openExposureFusion");
    qApp->setApplicationVersion("0.1");
    qApp->setApplicationDisplayName(QString("%1 x%2 v%3")
                                    .arg(qApp->applicationName())
                                    .arg(sizeof(void*) * 8)
                                    .arg(qApp->applicationVersion()));

    qmlRegisterType<ImageElement>("ImageElement", 1, 0, "ImageElement");
    qmlRegisterType<QmlHelper>("QmlHelper", 1, 0, "QmlHelper");

    qRegisterMetaType<QList<QImage>>("QList<QImage>");
    qRegisterMetaType<MertensCl::Parameters>("MertensCl::Parameters");
}

void MainController::initUi()
{
    mWnd = new MainWindow(this);
    mWnd->setProperty(MainWindow::PT_OutputFormatModel,     QVariant::fromValue(mFileFormatsModel));
    mWnd->setProperty(MainWindow::PT_InputFilesModel,       QVariant::fromValue(&mInputFilesModel));
    mWnd->setProperty(MainWindow::PT_DevicesModel,          QVariant::fromValue(&mDevicesModel));
    mWnd->setProperty(MainWindow::PT_DevicesPropertyModel,  QVariant::fromValue(&mDeviceInfoModel));
    mWnd->setProperty(MainWindow::PT_Title,                 qApp->applicationDisplayName());
}

void MainController::initCore()
{
    static const QStringList supported = {"bmp", "jpg", "jpeg", "png", "tif", "tiff"};
    const QList<QByteArray> available = QImageWriter::supportedImageFormats();
    for(int i = 0; i < available.count(); ++i)
    {
        const QString format = available.at(i).toLower();
        if(supported.contains(format))
        {
            mFileFormatsModel.append(format);
        }
    }
}

void MainController::initOpenCLDevices()
{
    const QList<ClPlatform> platforms = ClPlatform::getPlatforms();
    QList<ClDevice> finalDevices;
    for(int ipl = 0; ipl < platforms.count(); ++ipl)
    {
        const ClPlatform platform = platforms.at(ipl);
        qDebug() << platform;
        const QList<ClDevice> devices = platform.getDevices();
        for(int idev = 0; idev < devices.count(); ++idev)
        {
            const ClDevice device = devices.at(idev);
            if(device.isAvailable() && device.isCompilerAvailable() && device.areImagesSupported())
            {
                finalDevices.append(device);
            }
        }
    }
    mDevicesModel.setDevices(finalDevices);
}

bool MainController::initFusion()
{
    QVector<cl_context> contexts;
    for(int i = 0; i < mDevicesModel.rowCount(); ++i)
        contexts.append(mDevicesModel.at(i).getContext());
    if(!mExpoFusion.init(contexts))
        return false;

    mThreadForCore.start(QThread::LowPriority);
    mExpoFusion.moveToThread(&mThreadForCore);
    return true;
}

void MainController::connectSignals()
{
    connect(mWnd, SIGNAL(propertyChanged(MainWindow::PropertyType)),    SLOT(onPropertyChanged(MainWindow::PropertyType)));
    connect(mWnd, SIGNAL(saveClicked()),                                SLOT(onSaveClicked()));
    connect(mWnd, SIGNAL(updateViewClicked()),                          SLOT(onUpdateViewClicked()));

    connect(&mExpoFusion, SIGNAL(finished(QImage)), SLOT(onFinished(QImage)));

    connect(&mInputFilesModel, SIGNAL(rowsInserted(QModelIndex,int,int)),   SLOT(onInputListChanged()));
    connect(&mInputFilesModel, SIGNAL(rowsRemoved(QModelIndex,int,int)),    SLOT(onInputListChanged()));
}

void MainController::release()
{
    mThreadForCore.quit();
    if(!mThreadForCore.wait(1000))
    {
        mThreadForCore.terminate();
    }
}

void MainController::loadSettings()
{
    for(int i = 0; i < Settings::T_max; ++i)
    {
        const Settings::Type type = static_cast<Settings::Type>(i);
        const QVariant value = Settings::get(type, Settings::getDefault(type));
        const MainWindow::PropertyType propType = kMapOptionToProperty.value(type, MainWindow::PT_max);
        if(propType != MainWindow::PT_max)
        {
            mWnd->setProperty(propType, value);
        }
        else
        {
            switch(type)
            {
                case Settings::T_OutputFormat:
                {
                    const QString format = value.toString().toLower();
                    const int index = mFileFormatsModel.indexOf(format);
                    mWnd->setProperty(MainWindow::PT_OutputFormatIndex, qMax(0, index));
                    break;
                }
                default: break;
            }
        }
    }
}

void MainController::onPropertyChanged(const MainWindow::PropertyType type)
{
    const Settings::Type optionType = kMapOptionToProperty.key(type, Settings::T_max);
    const QVariant wndValue = mWnd->getProperty(type);
    if(optionType != Settings::T_max)
    {
        Settings::set(optionType, wndValue);
    }

    switch(type)
    {
        case MainWindow::PT_OutputFormatIndex:
            Settings::set(Settings::T_OutputFormat, mFileFormatsModel.at(wndValue.toInt()));
            break;

        case MainWindow::PT_DevicesIndex:
            mDeviceInfoModel.setDevice(mDevicesModel.at(wndValue.toInt()));
            mWnd->resizeDevicesTable();
            updateDeviceWarning();
            updateMemoryUsage();
            break;

        default: break;
    }

    static const QSet<MainWindow::PropertyType> updateTriggers = {
        MainWindow::PT_AutoUpdate,
        MainWindow::PT_MeasureContrast,
        MainWindow::PT_MeasureExposedness,
        MainWindow::PT_MeasureSaturation,
        MainWindow::PT_DevicesIndex
    };
    if(updateTriggers.contains(type))
    {
        if(mWnd->getProperty(MainWindow::PT_AutoUpdate).toBool())
        {
            processImages();
        }
    }
}

void MainController::onFinished(const QImage result)
{
    mIsProcessing = false;
    mWnd->setProperty(MainWindow::PT_ResultImage, result);
    mWnd->setProperty(MainWindow::PT_Progress, 0);

    killTimer(mProcessingTimerId);
    mProcessingTimerId = -1;

    qDebug() << "processing end" << QTime::currentTime().toString(kTimeFormat);

    if(mScheduleUpdate)
    {
        processImages();
    }
}

void MainController::processImages()
{
    if(mIsProcessing)
    {
        mScheduleUpdate = true;
        return;
    }

    MertensCl::Parameters params;
    params.contrast = mWnd->getProperty(MainWindow::PT_MeasureContrast).toFloat();
    params.saturation = mWnd->getProperty(MainWindow::PT_MeasureSaturation).toFloat();
    params.exposedness = mWnd->getProperty(MainWindow::PT_MeasureExposedness).toFloat();

    mExpoFusion.setCl(mDeviceInfoModel.getDevice().getContext(), mDeviceInfoModel.getDevice().getId());
    mExpoFusion.setParameters(params);
    QMetaObject::invokeMethod(&mExpoFusion, "process");

    mProcessingTime.restart();
    mProcessingTimerId = startTimer(1);
    mScheduleUpdate = false;
    mIsProcessing = true;
    mWnd->setProperty(MainWindow::PT_Progress, 1);
    mWnd->setProperty(MainWindow::PT_StatusText, QTime(0,0).addMSecs(mProcessingTime.elapsed()).toString(kTimeFormat));

    qDebug() << "processing start" << QTime::currentTime().toString(kTimeFormat);
}

void MainController::onInputListChanged()
{
    const QList<FileInfo> infos = mInputFilesModel.getFiles();
    QList<QImage> images;
    for(int i = 0; i < infos.count(); ++i)
    {
        images.append(infos.at(i).getImage());
    }
    QMetaObject::invokeMethod(&mExpoFusion, "setImages", Q_ARG(const QList<QImage>, images));

    if(mWnd->getProperty(MainWindow::PT_AutoUpdate).toBool())
    {
        processImages();
    }
    updateMemoryUsage();
}

void MainController::onSaveClicked()
{
    const QList<FileInfo> infos = mInputFilesModel.getFiles();
    if(infos.isEmpty())
        return;

    const QImage image = mWnd->getProperty(MainWindow::PT_ResultImage).value<QImage>();
    if(image.isNull())
        return;

    QStringList fileNames;
    for(int i = 0; i < infos.count(); ++i)
    {
        fileNames.append(QFileInfo(infos.at(i).getFilePath()).completeBaseName());
    }
    std::sort(fileNames.begin(), fileNames.end());
    const QString format = mFileFormatsModel.at(mWnd->getProperty(MainWindow::PT_OutputFormatIndex).toInt());
    const QString fileName = fileNames.first() + "-" + fileNames.last();
    const QString dir = QDir::toNativeSeparators(mWnd->getProperty(MainWindow::PT_OutputDir).toString());
    const QString fullpath = dir
                             + (dir.endsWith(QDir::separator()) ? QString() : QDir::separator())
                             + fileName
                             + "."
                             + format;
    image.save(fullpath);
}

void MainController::onUpdateViewClicked()
{
    processImages();
}

void MainController::updateDeviceWarning()
{
    const QList<ClDevice> devices = mDevicesModel.getDevices();
    bool hasGpu = false;
    for(int i = 0; i < devices.count(); ++i)
    {
        if(devices.at(i).getType() == CL_DEVICE_TYPE_GPU)
        {
            hasGpu = true;
            break;
        }
    }
    mWnd->setProperty(MainWindow::PT_DeviceWarningVisible,
                      hasGpu && (mDeviceInfoModel.getDevice().getType() != CL_DEVICE_TYPE_GPU));
}


void MainController::timerEvent(QTimerEvent *event)
{
    if(event->timerId() == mProcessingTimerId)
    {
        mWnd->setProperty(MainWindow::PT_StatusText, QTime(0,0).addMSecs(mProcessingTime.elapsed()).toString(kTimeFormat));
    }
}

void MainController::updateMemoryUsage()
{
    const qint64 deviceMem = mDeviceInfoModel.getDevice().getGlobalMemory();
    if(mInputFilesModel.isEmpty())
    {
        mWnd->setProperty(MainWindow::PT_MemoryProgress, 0);
        mWnd->setProperty(MainWindow::PT_MemoryText, tr("0 bytes of %1").arg(Util::toHumanText(deviceMem)));
    }
    else
    {
        const QList<FileInfo> files = mInputFilesModel.getFiles();
        const qint64 processMem = MertensCl::calcMemoryFootprint(files.first().getImage().size(),
                                                                 files.count());
        const double percent = (double)processMem / deviceMem;
        mWnd->setProperty(MainWindow::PT_MemoryProgress, percent * 100);
        mWnd->setProperty(MainWindow::PT_MemoryText, tr("%1 of %2")
                          .arg(Util::toHumanText(processMem))
                          .arg(Util::toHumanText(deviceMem)));
    }
}
