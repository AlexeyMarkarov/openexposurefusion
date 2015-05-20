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

#ifndef MERTENSCL_H
#define MERTENSCL_H

#include <QtCore>
#include <QtGui>
#include "wrappersCL/ClDevice.h"

class MertensCl : public QObject
{
    Q_OBJECT
    Q_ENUMS(KernelType)

public:
    enum KernelType
    {
        KT_Weight = 0,
        KT_Add,
        KT_Sub,
        KT_Div,
        KT_Mul,
        KT_Fill,
        KT_Upsample,
        KT_ToRgba,
        KT_Copy,
        KT_FilterGauss,
        KT_max
    };

    class Parameters
    {
    public:
        float contrast;
        float saturation;
        float exposedness;
    };

    static qint64 calcMemoryFootprint(const QSize imgSize, const int imgCount);

    MertensCl();
    ~MertensCl();

    bool init(const QVector<cl_context> contexts);

public slots:
    void setCl(const cl_context context, const cl_device_id device);
    void setImages(const QList<QImage> images);
    void setParameters(const MertensCl::Parameters params);
    QImage process();

    QImage process(const cl_context context, const cl_device_id device, const QList<QImage> sourceImages, const MertensCl::Parameters params);

signals:
    void finished(const QImage result)const;

private:
    enum ProcessingImage
    {
        PI_Result = 0,
        PI_TmpRHalf,
        PI_WeightSum,
        PI_max
    };

    class KernelInfo
    {
    public:
        cl_kernel kernel;
        size_t workSize;
        size_t preferredSize;

        KernelInfo(const cl_kernel k = 0, const size_t s = 0, const size_t ps = 0)
            : kernel(k), workSize(s), preferredSize(ps)
        { }

        inline friend QDebug operator <<(QDebug dbg, const KernelInfo &obj)
        {
            dbg.nospace() << "KernelInfo(" << "kernel=" << obj.kernel << " size=" << obj.workSize
                          << " preferred=" << obj.preferredSize << ")";
            return dbg.space();
        }
    };

    class Runtime
    {
    public:
        cl_program program;
        cl_command_queue queue;
        QMap<KernelType, KernelInfo> kernels;

        Runtime(const cl_program program_ = 0,
                const cl_command_queue queue_ = 0,
                const QMap<KernelType, KernelInfo> kernels_ = QMap<KernelType, KernelInfo>())
            : program(program_), queue(queue_), kernels(kernels_)
        { }

        bool isValid()const { return program && queue && (kernels.count() == KT_max); }
    };

    static const QMap<ProcessingImage, cl_image_format> sFormatsMap;

    static cl_program createProgram(const cl_context context);
    static bool buildProgram(const cl_program program, const cl_device_id device);
    static QMap<KernelType, KernelInfo> createKernels(const cl_program program, const cl_device_id device);
    static cl_command_queue createCommandQueue(const cl_context context, const cl_device_id device);
    static QList<QImage> resize(const QList<QImage> images, const cl_device_id device);
    static Runtime compile(const cl_context context, const cl_device_id device);
    static QVector<cl_mem> createImages(const cl_context context, const cl_mem_flags flags, const QList<QImage> images);
    static int calcPyrHeight(const QSize size);

    // persistent values
    cl_context mContext;
    cl_device_id mDevice;
    QMap<cl_context, QMap<cl_device_id, Runtime>> mRuntimes;
    Parameters mParams;
    QList<QImage> mImages;

    // processing values, have to be created if empty, and cleared when device or images change
    int mPyrHeight;
    size_t mMaxLocalGroupSize;
    size_t mMaxLocalGroupSizeSqrt;
    size_t mMaxLocalGroupSizes[2];
    QList<QImage> mCachedImages;
    QVector<cl_mem> mMemSrcImages;
    QVector<cl_mem> mMemProcessingImgs;
    QVector<cl_mem> mMemWeights;
    QVector<cl_mem> mMemResultPyramid;
    QVector<cl_mem> mMemWeightPyramid;
    QVector<cl_mem> mMemImagePyramid;
    QVector<cl_mem> mMemPyrRgbaHalf1;
    QVector<cl_mem> mMemPyrRgbaHalf2;
    QVector<QSize> mPyrSizes;

    QVector< QPair<cl_event, QString> > mProfile;

    void clearProcessingData();

    QImage assertAndProcess();
    bool allocProcessingImages();
    QImage process(const Runtime runtime, const QSize size);
    bool createWeightMap(const Runtime runtime, const cl_mem image, const QSize size,
                         const Parameters params, const cl_mem weightMap);
    bool normalizeWeights(const Runtime runtime, const QSize size);
    bool buildGaussPyr(const Runtime runtime, const QSize size, const cl_mem src,
                       const QVector<cl_mem> pyr, const QVector<cl_mem> tmpPyr);
    bool buildLaplacePyr(const Runtime runtime, const QVector<cl_mem> pyrSrc, const QVector<cl_mem> pyrDst,
                         const QVector<cl_mem> pyrTmp1, const QVector<cl_mem> pyrTmp2);
    bool multiresBlend(const Runtime runtime, const QSize size, const int imageIndex);
    bool mergeResultPyr(const Runtime runtime);
    QImage toImage(const Runtime runtime, const QSize size, const cl_mem mem);
    bool copy(const Runtime runtime, const QSize size, const cl_mem src, const cl_mem dst);
    void printProfilingInfo();
    bool filterGauss(const Runtime runtime, const cl_mem src, const cl_mem dst, const cl_mem tmp,
                     const QSize srcSize, const QSize dstSize,
                     const bool downScale, const cl_float4 factor);

    template<typename Arg>
    static cl_int setKernelArg(const cl_kernel kernel, const int argIndex, const Arg arg);

    template<typename Arg, typename ... Args>
    static cl_int setKernelArg(const cl_kernel kernel, const int argIndex, const Arg arg, const Args ... args);

    template<typename Arg, typename ... Args>
    cl_int enqueueKernel(const Runtime runtime, const KernelType type, const QSize size,
                         const Arg arg, const Args ... args);
};

Q_DECLARE_METATYPE(MertensCl::Parameters)

#endif // MERTENSCL_H
