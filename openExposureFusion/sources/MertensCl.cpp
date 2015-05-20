/*
OpenCL implementation of the Exposure Fusion
(algorithm created by Tom Mertens, Jan Kautz, Frank Van Reeth)

Copyright (c) 2015 Alexey Markarov

Permission is hereby granted, free of charge,
to any person obtaining a copy of this software
and associated documentation files (the "Software"),
to deal in the Software without restriction,
including without limitation the rights to use, copy,
modify, merge, publish, distribute, sublicense,
and/or sell copies of the Software,
and to permit persons to whom the Software is furnished to do so,
subject to the following conditions:

The above copyright notice and this permission notice
shall be included in all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO
THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.
IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS
BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY,
WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE,
ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE
OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
*/

#include "MertensCl.h"
#include "wrappersCL/ClProgram.h"
#include "Util.h"
#include <QtConcurrent>
#include <functional>

//#define PROFILING

#define MERTENSCL_ASSERT(errorCode, message, returnValue) \
    if(errorCode != CL_SUCCESS){ \
    qDebug() << errorCode << Util::toString(errorCode) << message; \
    return returnValue; \
    }

const cl_image_format kFormatRUnormInt8     = {CL_R,    CL_UNORM_INT8};
const cl_image_format kFormatRgbaUnormInt8  = {CL_RGBA, CL_UNORM_INT8};
const cl_image_format kFormatRHalf          = {CL_R,    CL_HALF_FLOAT};
const cl_image_format kFormatRgbaHalf       = {CL_RGBA, CL_HALF_FLOAT};

const QMap<MertensCl::ProcessingImage, cl_image_format> MertensCl::sFormatsMap = {
    {MertensCl::PI_Result,      kFormatRgbaUnormInt8},
    {MertensCl::PI_TmpRHalf,    kFormatRHalf},
    {MertensCl::PI_WeightSum,   kFormatRHalf}
};

qint64 MertensCl::calcMemoryFootprint(const QSize imgSize, const int imgCount)
{
    qint64 bytes = 0;

    // mMemSrcImages
    bytes += Util::byteCount(imgSize, kFormatRgbaUnormInt8) * imgCount;

    // mMemProcessingImgs
    for(int i = 0; i < PI_max; ++i)
    {
        const ProcessingImage type = static_cast<ProcessingImage>(i);
        bytes += Util::byteCount(imgSize, sFormatsMap.value(type));
    }

    // mMemWeights
    bytes += Util::byteCount(imgSize, kFormatRHalf) * imgCount;

    // pyramids
    const int pyrHeight = calcPyrHeight(imgSize);
    QSize tmpSize = imgSize;
    for(int i = 0; i < pyrHeight; ++i)
    {
        // mMemResultPyramid
        bytes += Util::byteCount(tmpSize, kFormatRgbaHalf);
        // mMemWeightPyramid
        bytes += Util::byteCount(tmpSize, kFormatRgbaHalf);
        // mMemImagePyramid
        bytes += Util::byteCount(tmpSize, kFormatRgbaHalf);
        // mMemPyrRgbaHalf1
        bytes += Util::byteCount(tmpSize, kFormatRgbaHalf);
        // mMemPyrRgbaHalf2
        bytes += Util::byteCount(tmpSize, kFormatRgbaHalf);
        tmpSize /= 2;
    }

    return bytes;
}

template<typename Arg>
cl_int MertensCl::setKernelArg(const cl_kernel kernel, const int argIndex, const Arg arg)
{
    const cl_int err = kernel ? clSetKernelArg(kernel, argIndex, sizeof(Arg), &arg) : CL_INVALID_KERNEL;
    MERTENSCL_ASSERT(err, QString("error setting kernel arg %1").arg(argIndex), err);
    return err;
}

template<typename Arg, typename ... Args>
cl_int MertensCl::setKernelArg(const cl_kernel kernel, const int argIndex, const Arg arg, const Args ... args)
{
    const cl_int err = setKernelArg(kernel, argIndex, arg);
    MERTENSCL_ASSERT(err, "kernel arg error", err);
    return setKernelArg(kernel, argIndex + 1, args...);
}

template<typename Arg, typename ... Args>
cl_int MertensCl::enqueueKernel(const Runtime runtime, const MertensCl::KernelType type, const QSize size,
                                const Arg arg, const Args ... args)
{
    static const QMetaEnum ktEnum = staticMetaObject.enumerator(staticMetaObject.indexOfEnumerator("KernelType"));
    const KernelInfo info = runtime.kernels.value(type);
    const QString kernelName = ktEnum.valueToKey(type);

    const cl_int2 kernelSize = {size.width(), size.height()};
    cl_int err = info.kernel ? clSetKernelArg(info.kernel, 0, sizeof(cl_int2), &kernelSize) : CL_INVALID_KERNEL;
    MERTENSCL_ASSERT(err, "error setting kernel size arg", err);

    err = setKernelArg(info.kernel, 1, arg, args...);
    MERTENSCL_ASSERT(err, "error setting args of kernel " + kernelName, err);

    size_t localSize[2];
    localSize[0] = std::min<size_t>(size.width(), info.preferredSize > 0 ? info.preferredSize : mMaxLocalGroupSizeSqrt);
    const size_t h = info.preferredSize > 0 ? info.preferredSize : mMaxLocalGroupSizeSqrt;
    localSize[1] = std::min<size_t>(size.height(),
                                    h * h <= mMaxLocalGroupSize ? h : mMaxLocalGroupSize / localSize[0]);

    const size_t globalSize[2] = {Util::addPadding(size.width(), localSize[0]),
                                  Util::addPadding(size.height(), localSize[1])};
#ifdef PROFILING
    qDebug() << kernelName << size << "global" << globalSize[0] << globalSize[1] << "local" << localSize[0] << localSize[1];
#endif

#ifdef PROFILING
    cl_event event;
#endif
    err = clEnqueueNDRangeKernel(runtime.queue, info.kernel, 2, nullptr, globalSize, localSize, 0, nullptr,
                             #ifdef PROFILING
                                 &event
                             #else
                                 nullptr
                             #endif
                                 );
    MERTENSCL_ASSERT(err, "unable to execute kernel " + kernelName, err);

#ifdef PROFILING
    mProfile.append({event, kernelName});
#endif
    return err;
}

MertensCl::MertensCl()
    : mContext(0),
      mDevice(0),
      mParams({1,1,0})
{
}

MertensCl::~MertensCl()
{
}

bool MertensCl::init(const QVector<cl_context> contexts)
{
    for(int ictx = 0; ictx < contexts.count(); ++ictx)
    {
        const cl_context context = contexts.at(ictx);
        size_t size = 0;
        clGetContextInfo(context, CL_CONTEXT_DEVICES, size, nullptr, &size);
        const int devicesCount = size / sizeof(cl_device_id);
        QScopedArrayPointer<cl_device_id> devices(new cl_device_id[devicesCount]);
        clGetContextInfo(context, CL_CONTEXT_DEVICES, size, devices.data(), nullptr);
        for(int idev = 0; idev < devicesCount; ++idev)
        {
            const cl_device_id device = devices[idev];
            mRuntimes[context][device] = compile(context, device);
        }
    }
    return true;
}

void MertensCl::setCl(const cl_context context, const cl_device_id device)
{
    qDebug() << "setCl" << context << device << "current" << mContext << mDevice;
    if(mContext == context && mDevice == device)
    {
        // do nothing
    }
    else
    {
        mContext = context;
        mDevice = device;
        clearProcessingData();
    }
}

void MertensCl::setImages(const QList<QImage> images)
{
    mImages = images;
    clearProcessingData();
}

void MertensCl::setParameters(const MertensCl::Parameters params)
{
    mParams = params;
}

QImage MertensCl::process()
{
    const QImage result = assertAndProcess();
    emit finished(result);
    return result;
}

QImage MertensCl::process(const cl_context context, const cl_device_id device, const QList<QImage> sourceImages, const MertensCl::Parameters params)
{
    setCl(context, device);
    setImages(sourceImages);
    setParameters(params);
    return process();
}

MertensCl::Runtime MertensCl::compile(const cl_context context, const cl_device_id device)
{
    qDebug() << "compile for" << context << device;
    const QString deviceName = ClDevice::getDeviceName(device);
    qDebug() << "device is" << deviceName;

    const cl_program program = createProgram(context);
    if(!program)
    {
        qDebug() << "unable to create the program";
        return Runtime();
    }

    if(!buildProgram(program, device))
    {
        qDebug() << "unable to build the program";
        return Runtime();
    }

    const QMap<KernelType, KernelInfo> kernels = createKernels(program, device);
    qDebug() << "created kernels" << kernels;
    if(kernels.count() != KT_max)
    {
        qDebug() << "not all kernels are created";
        return Runtime();
    }

    const cl_command_queue queue = createCommandQueue(context, device);
    if(!queue)
    {
        qDebug() << "unable to create the queue";
        return Runtime();
    }

    return Runtime(program, queue, kernels);
}

cl_program MertensCl::createProgram(const cl_context context)
{
    static const QString sourceFilePath(":/mertens.cl");

    QFile file(sourceFilePath);
    const QByteArray sources = file.open(QFile::ReadOnly) ? file.readAll() : QByteArray();
    file.close();

    qDebug() << "source code length" << sources.length();
    if(sources.isEmpty())
        return 0;

    cl_int errorCode = CL_SUCCESS;

    const char *src = sources.constData();
    const size_t len = sources.length();
    const cl_program program = clCreateProgramWithSource(context, 1, &src, &len, &errorCode);

    qDebug() << "program" << program << "errorCode" << errorCode << Util::toString(errorCode);
    return program;
}

bool MertensCl::buildProgram(const cl_program program, const cl_device_id device)
{
    const cl_int buildResult = clBuildProgram(program, 1, &device, nullptr, nullptr, nullptr);
    const cl_build_status buildStatus = ClProgram::getProgramBuildStatus(program, device);
    const QStringList buildLog = ClProgram::getProgramBuildLog(program, device);

    qDebug() << "build result" << buildResult << Util::toString(buildResult)
             << "\nbuild status" << buildStatus << Util::toStatusString(buildStatus)
             << "\nbuild log" << buildLog.join("\n");

    return buildResult == CL_SUCCESS;
}

QMap<MertensCl::KernelType, MertensCl::KernelInfo> MertensCl::createKernels(const cl_program program,
                                                                            const cl_device_id device)
{
    static const QMap<KernelType, QByteArray> kernelNames = {
        {KT_Weight,         "krn_weight"},
        {KT_Add,            "krn_add"},
        {KT_Sub,            "krn_sub"},
        {KT_Div,            "krn_div"},
        {KT_Mul,            "krn_mul"},
        {KT_Fill,           "krn_fill"},
        {KT_Upsample,       "krn_upsample"},
        {KT_ToRgba,         "krn_toRgba"},
        {KT_Copy,           "krn_copy"},
        {KT_FilterGauss,    "krn_filterGauss"}
    };

    QMap<KernelType, KernelInfo> kernels;
    cl_int errorCode;

    for(int i = 0; i < KT_max; ++i)
    {
        const KernelType type = static_cast<KernelType>(i);
        const QByteArray name = kernelNames.value(type);
        qDebug() << "creating kernel" << type << name;
        if(name.isEmpty())
        {
            qDebug() << "unknown kernel, name is missing";
            continue;
        }
        const cl_kernel kernel = clCreateKernel(program, name.constData(), &errorCode);
        qDebug() << "created kernel" << kernel << "error" << errorCode << Util::toString(errorCode);
        if(!kernel || errorCode != CL_SUCCESS)
        {
            qDebug() << "clCreateKernel failed";
            continue;
        }

        size_t workSize;
        errorCode = clGetKernelWorkGroupInfo(kernel, device, CL_KERNEL_WORK_GROUP_SIZE,
                                             sizeof(size_t), &workSize, nullptr);
        if(errorCode != CL_SUCCESS)
        {
            qDebug() << "CL_KERNEL_WORK_GROUP_SIZE failed";
            workSize = 0;
        }

        size_t preferredSize;
        errorCode = clGetKernelWorkGroupInfo(kernel, device, CL_KERNEL_PREFERRED_WORK_GROUP_SIZE_MULTIPLE,
                                                     sizeof(size_t), &preferredSize, nullptr);
        if(errorCode != CL_SUCCESS)
        {
            qDebug() << "CL_KERNEL_PREFERRED_WORK_GROUP_SIZE_MULTIPLE failed";
            preferredSize = 0;
        }

        kernels[type] = KernelInfo(kernel, workSize, preferredSize);
    }

    return kernels;
}

cl_command_queue MertensCl::createCommandQueue(const cl_context context, const cl_device_id device)
{
    cl_int errorCode = CL_SUCCESS;
    const cl_command_queue queue = clCreateCommandQueue(context, device,
                                                    #ifdef PROFILING
                                                        CL_QUEUE_PROFILING_ENABLE,
                                                    #else
                                                        0,
                                                    #endif
                                                        &errorCode);
    qDebug() << "created queue" << queue << errorCode << Util::toString(errorCode);
    return queue;
}

QList<QImage> MertensCl::resize(const QList<QImage> images, const cl_device_id device)
{
    qDebug() << "resize images" << images;
    if(images.isEmpty())
        return images;

    QSize minSize = images.first().size();
    for(int i = 1; i < images.count(); ++i)
    {
        const QSize s(images.at(i).size());
        minSize = QSize(std::min(minSize.width(), s.width()),
                        std::min(minSize.height(), s.height()));
    }

    const QSize supportedSize = ClDevice::getDeviceImageSize(device);
    qDebug() << "supportedSize" << supportedSize;
    const QSize newSize = minSize.boundedTo(supportedSize);
    qDebug() << "newSize" << newSize;

    const std::function<QImage (const QImage&)> mapFunctor =
            [newSize](const QImage &img) -> QImage
    {
        return img.scaled(newSize, Qt::IgnoreAspectRatio, Qt::SmoothTransformation)
                .convertToFormat(QImage::Format_RGBA8888);
    };
    static const std::function<void (QList<QImage>&, const QImage&)> reduceFunctor =
            [](QList<QImage> &all, const QImage &one) -> void
    {
        if(!one.isNull()) all.append(one);
    };
    const QList<QImage> result = QtConcurrent::blockingMappedReduced<QList<QImage>>(images, mapFunctor, reduceFunctor);

    return result;
}

QVector<cl_mem> MertensCl::createImages(const cl_context context, const cl_mem_flags flags, const QList<QImage> images)
{
    cl_int error;
    QVector<cl_mem> mems;
    for(int i = 0; i < images.count(); ++i)
    {
        const QImage image(images.at(i));
        if(image.isNull())
            continue;

        const cl_mem obj = clCreateImage2D(context,
                                           flags | CL_MEM_USE_HOST_PTR,
                                           &kFormatRgbaUnormInt8,
                                           image.width(),
                                           image.height(),
                                           0,
                                           const_cast<uchar*>(image.constBits()),
                                           &error);
        qDebug() << "created image" << obj << error << Util::toString(error);
        if(obj)
        {
            mems.append(obj);
        }
    }
    return mems;
}

int MertensCl::calcPyrHeight(const QSize size)
{
    return logf(std::min(size.width(), size.height())) / logf(2.0);
}

QImage MertensCl::assertAndProcess()
{
    if(!mContext || !mDevice || mImages.isEmpty())
        return QImage();

    Runtime runtime = mRuntimes.value(mContext).value(mDevice);
    if(!runtime.isValid())
    {
        qDebug() << "missing runtime";
        runtime = compile(mContext, mDevice);
        mRuntimes[mContext][mDevice] = runtime;
    }
    if(!runtime.isValid())
    {
        qDebug() << "unable to compile runtime objects";
        return QImage();
    }

    bool areImagesReady = mCachedImages.count() == mImages.count();
    if(!areImagesReady)
    {
        qDebug() << "images are not ready";
        areImagesReady = allocProcessingImages();
    }
    if(!areImagesReady)
    {
        qDebug() << "can't load or create images";
        clearProcessingData();
        return QImage();
    }

    mMaxLocalGroupSize = ClDevice::getDeviceMaxWorkGroupSize(mDevice);
    mMaxLocalGroupSizeSqrt = qSqrt(mMaxLocalGroupSize);
    const QVector<size_t> sizes = ClDevice::getDeviceMaxWorkItemSizes(mDevice);
    if(sizes.count() < 2)
    {
        qDebug() << "unable to get max local work sizes";
        clearProcessingData();
        return QImage();
    }
    mMaxLocalGroupSizes[0] = sizes[0];
    mMaxLocalGroupSizes[1] = sizes[1];

    mProfile.clear();

    return process(runtime, mCachedImages.first().size());
}

bool MertensCl::allocProcessingImages()
{
    mCachedImages = resize(mImages, mDevice);
    if(mCachedImages.count() != mImages.count())
    {
        qDebug() << "unable to cache images";
        return false;
    }

    mMemSrcImages = createImages(mContext, CL_MEM_READ_ONLY, mCachedImages);
    if(mMemSrcImages.count() != mCachedImages.count())
    {
        qDebug() << "unable to load images into textures";
        return false;
    }

    const QSize size = mCachedImages.first().size();
    cl_int error;

    for(int i = 0; i < PI_max; ++i)
    {
        const ProcessingImage type = static_cast<ProcessingImage>(i);
        const cl_image_format format = sFormatsMap.value(type, {0, 0});
        const cl_mem img = clCreateImage2D(mContext,
                                           CL_MEM_READ_WRITE,
                                           &format,
                                           size.width(),
                                           size.height(),
                                           0,
                                           nullptr,
                                           &error);
        qDebug() << "created img" << type << img << error << Util::toString(error);
        if(img && (error == CL_SUCCESS))
        {
            mMemProcessingImgs.append(img);
        }
    }
    if(mMemProcessingImgs.count() != PI_max)
    {
        qDebug() << "unable to allocate temporary textures";
        return false;
    }

    for(int i = 0; i < mCachedImages.count(); ++i)
    {
        const cl_mem img = clCreateImage2D(mContext,
                                           CL_MEM_READ_WRITE,
                                           &kFormatRHalf,
                                           size.width(),
                                           size.height(),
                                           0,
                                           nullptr,
                                           &error);
        qDebug() << "created weight" << img << error << Util::toString(error);
        if(img && (error == CL_SUCCESS))
        {
            mMemWeights.append(img);
        }
    }
    if(mMemWeights.count() != mCachedImages.count())
    {
        qDebug() << "unable to allocate weight textures";
        return false;
    }

    mPyrHeight = calcPyrHeight(size);
    qDebug() << "pyramids height" << mPyrHeight;
    QSize tmpSize = size;
    for(int i = 0; i < mPyrHeight; ++i)
    {
        mPyrSizes.append(tmpSize);
        {
            const cl_mem img = clCreateImage2D(mContext,
                                               CL_MEM_READ_WRITE,
                                               &kFormatRgbaHalf,
                                               tmpSize.width(),
                                               tmpSize.height(),
                                               0,
                                               nullptr,
                                               &error);
            qDebug() << "created img pyr" << tmpSize << img << error << Util::toString(error);
            if(img && (error == CL_SUCCESS))
                mMemImagePyramid.append(img);
        }
        {
            const cl_mem weight = clCreateImage2D(mContext,
                                                  CL_MEM_READ_WRITE,
                                                  &kFormatRgbaHalf,
                                                  tmpSize.width(),
                                                  tmpSize.height(),
                                                  0,
                                                  nullptr,
                                                  &error);
            qDebug() << "created weight pyr" << tmpSize << weight << error << Util::toString(error);
            if(weight && (error == CL_SUCCESS))
                mMemWeightPyramid.append(weight);
        }
        {
            const cl_mem result = clCreateImage2D(mContext,
                                                  CL_MEM_READ_WRITE,
                                                  &kFormatRgbaHalf,
                                                  tmpSize.width(),
                                                  tmpSize.height(),
                                                  0,
                                                  nullptr,
                                                  &error);
            qDebug() << "created result pyr" << tmpSize << result << error << Util::toString(error);
            if(result && (error == CL_SUCCESS))
                mMemResultPyramid.append(result);
        }
        {
            const cl_mem img = clCreateImage2D(mContext,
                                               CL_MEM_READ_WRITE,
                                               &kFormatRgbaHalf,
                                               tmpSize.width(),
                                               tmpSize.height(),
                                               0,
                                               nullptr,
                                               &error);
            qDebug() << "created rgbahalf pyr" << tmpSize << img << error << Util::toString(error);
            if(img && (error == CL_SUCCESS))
                mMemPyrRgbaHalf1.append(img);
        }
        {
            const cl_mem img = clCreateImage2D(mContext,
                                               CL_MEM_READ_WRITE,
                                               &kFormatRgbaHalf,
                                               tmpSize.width(),
                                               tmpSize.height(),
                                               0,
                                               nullptr,
                                               &error);
            qDebug() << "created rgbahalf pyr2" << tmpSize << img << error << Util::toString(error);
            if(img && (error == CL_SUCCESS))
                mMemPyrRgbaHalf2.append(img);
        }
        tmpSize /= 2;
    }
    if((mMemImagePyramid.count()        != mPyrHeight)
       || (mMemWeightPyramid.count()    != mPyrHeight)
       || (mMemResultPyramid.count()    != mPyrHeight)
       || (mMemPyrRgbaHalf1.count()     != mPyrHeight)
       || (mMemPyrRgbaHalf2.count()     != mPyrHeight))
    {
        qDebug() << "unable to allocate temporary pyramids";
        return false;
    }

    return true;
}

QImage MertensCl::process(const Runtime runtime, const QSize size)
{
    if(!runtime.isValid() || size.isEmpty())
        return QImage();

    //===== Create Weights
    for(int i = 0; i < mMemSrcImages.count(); ++i)
    {
        if(!createWeightMap(runtime, mMemSrcImages.at(i), size, mParams, mMemWeights.at(i)))
        {
            qDebug() << "unable to create weight map";
            return QImage();
        }
    }

    //===== Clear Weights sum
    static const cl_float4 float4Zeros = {0.0f, 0.0f, 0.0f, 0.0f};
    MERTENSCL_ASSERT(enqueueKernel(runtime, KT_Fill, size, float4Zeros, mMemProcessingImgs.at(PI_WeightSum)),
                     "unable to clear weights sum map",
                     QImage());

    //===== Normalize Weights
    if(!normalizeWeights(runtime, size))
    {
        qDebug() << "unable to normalize weights";
        return QImage();
    }

    //===== Clear Result pyramid
    for(int i = 0; i < mPyrHeight; ++i)
    {
        MERTENSCL_ASSERT(enqueueKernel(runtime, KT_Fill, mPyrSizes.at(i), float4Zeros, mMemResultPyramid.at(i)),
                         "unable to clear result pyramid",
                         QImage());
    }

    //===== Multiresolution blend
    for(int i = 0; i < mMemSrcImages.count(); ++i)
    {
        if(!multiresBlend(runtime, size, i))
        {
            qDebug() << "unable to blend image #" << i;
            return QImage();
        }
    }

    if(!mergeResultPyr(runtime))
    {
        qDebug() << "unable to reconstruct result pyramid";
        return QImage();
    }

    MERTENSCL_ASSERT(enqueueKernel(runtime, KT_ToRgba, size,
                                   mMemResultPyramid.at(0), mMemProcessingImgs.at(PI_Result)),
                     "unable to convert final image",
                     QImage());

    qDebug() << "read result";
    const QImage img = toImage(runtime, size, mMemProcessingImgs.at(PI_Result));

    printProfilingInfo();
    return img;
}

bool MertensCl::createWeightMap(const Runtime runtime, const cl_mem image, const QSize size,
                                const Parameters params, const cl_mem weightMap)
{
    if(!runtime.isValid() || size.isEmpty())
        return false;

    const cl_float3 clparams = {params.contrast, params.saturation, params.exposedness};
    const cl_int2 maxCoord = {size.width() - 1, size.height() - 1};
    MERTENSCL_ASSERT(enqueueKernel(runtime, KT_Weight, size, image, weightMap, clparams, maxCoord),
                     "unable to create weight map",
                     false);
    return true;
}

bool MertensCl::normalizeWeights(const Runtime runtime, const QSize size)
{
    if(!runtime.isValid() || size.isEmpty())
        return false;

    //===== Sum Weights
    for(int i = 0; i < mMemWeights.count(); ++i)
    {
        MERTENSCL_ASSERT(enqueueKernel(runtime, KT_Add, size,
                                       mMemWeights.at(i),
                                       mMemProcessingImgs.at(PI_WeightSum),
                                       mMemProcessingImgs.at(PI_TmpRHalf)),
                         "unable to add weights",
                         false);
        std::swap(mMemProcessingImgs[PI_TmpRHalf], mMemProcessingImgs[PI_WeightSum]);
    }

    //===== Normalize Weights
    for(int i = 0; i < mMemWeights.count(); ++i)
    {
        MERTENSCL_ASSERT(enqueueKernel(runtime, KT_Div, size,
                                       mMemWeights.at(i),
                                       mMemProcessingImgs.at(PI_WeightSum),
                                       mMemProcessingImgs.at(PI_TmpRHalf)),
                         "unable to normalize weights",
                         false);
        std::swap(mMemWeights[i], mMemProcessingImgs[PI_TmpRHalf]);
    }

    return true;
}

bool MertensCl::buildGaussPyr(const Runtime runtime, const QSize size, const cl_mem src,
                              const QVector<cl_mem> pyr, const QVector<cl_mem> tmpPyr)
{
    if(!runtime.isValid() || size.isEmpty() || pyr.isEmpty())
        return false;

    if(!copy(runtime, size, src, pyr.first()))
    {
        qDebug() << "unable to copy src image into pyr 0th level";
        return false;
    }

    //===== Apply Gauss blur and downsample
    for(int i = 0; i < (mPyrHeight - 1); ++i)
    {
        static const cl_float4 factor = {1.0f, 1.0f, 1.0f, 1.0f};
        if(!filterGauss(runtime, pyr.at(i), pyr.at(i + 1), tmpPyr.at(i), mPyrSizes.at(i), mPyrSizes.at(i + 1),
                        true, factor))
        {
            qDebug() << "unable to apply gauss filter";
            return false;
        }
    }

    return true;
}

bool MertensCl::buildLaplacePyr(const Runtime runtime,
                                const QVector<cl_mem> pyrSrc, const QVector<cl_mem> pyrDst,
                                const QVector<cl_mem> pyrTmp1, const QVector<cl_mem> pyrTmp2)
{
    if(!runtime.isValid())
        return false;

    for(int i = 0; i < (mPyrHeight - 1); ++i)
    {
        const QSize bigSize = mPyrSizes.at(i);
        const QSize smallSize = mPyrSizes.at(i + 1);
        const cl_int2 maxCoord = {bigSize.width() - 1, bigSize.height() - 1};

        MERTENSCL_ASSERT(enqueueKernel(runtime, KT_Upsample, smallSize,
                                       pyrSrc.at(i + 1), pyrTmp1.at(i), maxCoord),
                         QString("unable to upsample for laplace pyramid %1").arg(i),
                         false);

        static const cl_float4 upsampleFactor = {4.0f, 4.0f, 4.0f, 4.0f};
        if(!filterGauss(runtime, pyrTmp1.at(i), pyrTmp2.at(i), pyrDst.at(i), bigSize, bigSize, false, upsampleFactor))
        {
            qDebug() << "unable to apply gauss for laplace pyramid" << i;
            return false;
        }

        MERTENSCL_ASSERT(enqueueKernel(runtime, KT_Sub, bigSize,
                                       pyrSrc.at(i), pyrTmp2.at(i), pyrDst.at(i)),
                         QString("unable to build laplace pyramid %1").arg(i),
                         false);
    }

    if(!copy(runtime, mPyrSizes.last(), pyrSrc.last(), pyrDst.last()))
    {
        qDebug() << "unable to copy the last level of src image into laplace pyr";
        return false;
    }

    return true;
}

bool MertensCl::multiresBlend(const Runtime runtime, const QSize size, const int imageIndex)
{
    if(!runtime.isValid() || size.isEmpty() || (imageIndex < 0) || (imageIndex >= mCachedImages.count()))
        return false;

    if(!buildGaussPyr(runtime, size, mMemSrcImages.at(imageIndex), mMemImagePyramid, mMemPyrRgbaHalf1))
    {
        qDebug() << "unable to create gauss pyr for image" << imageIndex;
        return false;
    }

    if(!buildLaplacePyr(runtime, mMemImagePyramid, mMemPyrRgbaHalf2, mMemPyrRgbaHalf1, mMemWeightPyramid))
    {
        qDebug() << "unable to create laplace pyr for image" << imageIndex;
        return false;
    }

    if(!buildGaussPyr(runtime, size, mMemWeights.at(imageIndex), mMemWeightPyramid, mMemPyrRgbaHalf1))
    {
        qDebug() << "unable to create gauss pyr for weight" << imageIndex;
        return false;
    }

    for(int i = 0; i < mPyrHeight; ++i)
    {
        MERTENSCL_ASSERT(enqueueKernel(runtime, KT_Mul, mPyrSizes.at(i),
                                       mMemPyrRgbaHalf2.at(i), mMemWeightPyramid.at(i), mMemPyrRgbaHalf1.at(i)),
                         QString("unable to multiply pyramids for image %1").arg(imageIndex),
                         false);
        MERTENSCL_ASSERT(enqueueKernel(runtime, KT_Add, mPyrSizes.at(i),
                                       mMemResultPyramid.at(i), mMemPyrRgbaHalf1.at(i), mMemPyrRgbaHalf2.at(i)),
                         QString("unable to add pyramids for image %1").arg(imageIndex),
                         false);
        std::swap(mMemResultPyramid[i], mMemPyrRgbaHalf2[i]);
    }

    return true;
}

bool MertensCl::mergeResultPyr(const Runtime runtime)
{
    if(!runtime.isValid())
        return false;

    for(int i = (mPyrHeight - 1); i > 0; --i)
    {
        const QSize smallSize = mPyrSizes.at(i);
        const QSize bigSize = mPyrSizes.at(i - 1);
        const cl_int2 maxCoord = {bigSize.width() - 1, bigSize.height() - 1};

        MERTENSCL_ASSERT(enqueueKernel(runtime, KT_Upsample, smallSize,
                                       mMemResultPyramid.at(i), mMemPyrRgbaHalf1.at(i - 1), maxCoord),
                         QString("unable to upsample result pyramid lvl %1").arg(i),
                         false);

        static const cl_float4 upsampleFactor = {4.0f, 4.0f, 4.0f, 4.0f};
        if(!filterGauss(runtime, mMemPyrRgbaHalf1.at(i - 1), mMemPyrRgbaHalf2.at(i - 1), mMemImagePyramid.at(i - 1),
                        bigSize, bigSize, false, upsampleFactor))
        {
            qDebug() << "unable to blur result pyramid lvl" << i;
            return false;
        }

        MERTENSCL_ASSERT(enqueueKernel(runtime, KT_Add, bigSize,
                                       mMemResultPyramid.at(i - 1),
                                       mMemPyrRgbaHalf2.at(i - 1),
                                       mMemPyrRgbaHalf1.at(i - 1)),
                         QString("unable to add result pyr at level %1").arg(i),
                         false);

        std::swap(mMemResultPyramid[i - 1], mMemPyrRgbaHalf1[i - 1]);
    }

    return true;
}

void MertensCl::clearProcessingData()
{
    Util::release(mMemSrcImages
                  + mMemProcessingImgs
                  + mMemWeights
                  + mMemResultPyramid
                  + mMemWeightPyramid
                  + mMemImagePyramid
                  + mMemPyrRgbaHalf1
                  + mMemPyrRgbaHalf2);

    mPyrHeight = -1;
    mMaxLocalGroupSize = -1;
    mMaxLocalGroupSizeSqrt = -1;
    mCachedImages.clear();
    mMemSrcImages.clear();
    mMemProcessingImgs.clear();
    mMemWeights.clear();
    mMemResultPyramid.clear();
    mMemWeightPyramid.clear();
    mMemImagePyramid.clear();
    mMemPyrRgbaHalf1.clear();
    mMemPyrRgbaHalf2.clear();
    mPyrSizes.clear();
    mProfile.clear();
}

QImage MertensCl::toImage(const Runtime runtime, const QSize size, const cl_mem mem)
{
    const size_t origin[] = {0, 0, 0};
    const size_t region[] = {static_cast<size_t>(size.width()), static_cast<size_t>(size.height()), 1};
    QScopedArrayPointer<uchar> buffer(new uchar[size.width() * size.height() * 4]);
#ifdef PROFILING
    cl_event event;
#endif
    MERTENSCL_ASSERT(clEnqueueReadImage(runtime.queue,
                                        mem,
                                        CL_TRUE,
                                        origin,
                                        region,
                                        0,
                                        0,
                                        buffer.data(),
                                        0,
                                        nullptr,
                                    #ifdef PROFILING
                                        &event
                                    #else
                                        nullptr
                                    #endif
                                        ),
                     "unable to read image",
                     QImage());
#ifdef PROFILING
    mProfile.append({event, "clEnqueueReadImage"});
#endif

    uchar *ptr = buffer.take();
    return QImage(ptr, size.width(), size.height(), QImage::Format_RGBA8888,
                  [](void *data){delete[] static_cast<uchar*>(data);}, ptr);
}

bool MertensCl::copy(const Runtime runtime, const QSize size, const cl_mem src, const cl_mem dst)
{
    if(!runtime.isValid() || size.isEmpty())
        return false;

    cl_image_format srcFormat;
    MERTENSCL_ASSERT(clGetImageInfo(src, CL_IMAGE_FORMAT, sizeof(cl_image_format), &srcFormat, nullptr),
                     "unable to get src image format",
                     false);

    cl_image_format dstFormat;
    MERTENSCL_ASSERT(clGetImageInfo(dst, CL_IMAGE_FORMAT, sizeof(cl_image_format), &dstFormat, nullptr),
                     "unable to get dst image format",
                     false);

    const bool areFormatsEqual = (srcFormat.image_channel_data_type == dstFormat.image_channel_data_type)
                                 && (srcFormat.image_channel_order == dstFormat.image_channel_order);
    if(areFormatsEqual)
    {
        const size_t origin[] = {0, 0, 0};
        const size_t region[] = {static_cast<size_t>(size.width()), static_cast<size_t>(size.height()), 1};
#ifdef PROFILING
        cl_event event;
#endif
        MERTENSCL_ASSERT(clEnqueueCopyImage(runtime.queue,
                                            src,
                                            dst,
                                            origin,
                                            origin,
                                            region,
                                            0,
                                            nullptr,
                                    #ifdef PROFILING
                                            &event
                                    #else
                                            nullptr
                                    #endif
                                            ),
                         "unable to copy image",
                         false);
#ifdef PROFILING
        mProfile.append({event, "clEnqueueCopyImage"});
#endif
    }
    else
    {
        MERTENSCL_ASSERT(enqueueKernel(runtime, KT_Copy, size, src, dst),
                         "unable to copy image",
                         false)
    }
    return true;
}

void MertensCl::printProfilingInfo()
{
#ifdef PROFILING
    qDebug() << "profiling info";
    qint64 sum = 0;
    for(int i = 0; i < mProfile.count(); ++i)
    {
        const QPair<cl_event, QString> info = mProfile.at(i);
        cl_ulong start, end;
        clGetEventProfilingInfo(info.first, CL_PROFILING_COMMAND_START, sizeof(cl_ulong), &start, nullptr);
        clGetEventProfilingInfo(info.first, CL_PROFILING_COMMAND_END, sizeof(cl_ulong), &end, nullptr);
        qDebug() << info.second
                 << "\tstart" << start
                 << "\tend" << end
                 << "\tduration" << (end - start)
                 << "\tduration msec" << (end - start) / 1e6;
        sum += end - start;
    }
    qDebug() << "total duration" << sum << sum / 1e6;
#endif
}

bool MertensCl::filterGauss(const Runtime runtime, const cl_mem src, const cl_mem dst, const cl_mem tmp,
                            const QSize srcSize, const QSize dstSize,
                            const bool downScale, const cl_float4 factor)
{
    if(!runtime.isValid())
        return false;

    // horizontal blur
    {
        const cl_int8 options = {srcSize.width() - 1, srcSize.height() - 1,
                                 downScale ? 2 : 1, 1,
                                 downScale ? 2 : 1, 1,
                                 1, 0};
        static const cl_float4 noFactor = {1.0f, 1.0f, 1.0f, 1.0f};
        MERTENSCL_ASSERT(enqueueKernel(runtime, KT_FilterGauss,
                                       QSize((downScale ? dstSize.width() : srcSize.width()), srcSize.height()),
                                       src, tmp, options, noFactor),
                         "unable to apply horizontal gauss filter",
                         false);
    }
    // vertical blur
    {
        const cl_int8 options = {srcSize.width() - 1, srcSize.height() - 1,
                                 downScale ? 2 : 1, downScale ? 2 : 1,
                                 1, 1,
                                 0, 1};
        MERTENSCL_ASSERT(enqueueKernel(runtime, KT_FilterGauss, dstSize,
                                       tmp, dst, options, factor),
                         "unable to apply vertical gauss filter",
                         false);
    }
    return true;
}
