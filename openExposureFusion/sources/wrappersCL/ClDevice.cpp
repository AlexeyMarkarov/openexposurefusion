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

#include "ClDevice.h"
#include "Util.h"

QDebug operator<<(QDebug dbg, const cl_image_format &format)
{
    dbg.nospace() << Util::toString(format);
    return dbg.space();
}

QDebug operator<<(QDebug dbg, const QVector<size_t> &vec)
{
    dbg.nospace() << "QVector(" << vec.count() << "){";
    for(int i = 0; i < vec.count(); ++i)
    {
        dbg.nospace() << vec.at(i);
        if(i != vec.count() - 1)
            dbg.nospace() << ", ";
    }
    dbg.nospace() << "}";
    return dbg.space();
}

QDebug operator<<(QDebug dbg, const ClDevice &dev)
{
    dbg.nospace() << "\nClDevice{"
                  << "\n\tid\t= "               << dev.getId()
                  << "\n\tname\t= "             << dev.getName()
                  << "\n\tbits\t= "             << dev.getBits()
                  << "\n\tmemory\t= "           << dev.getGlobalMemory() << " " << Util::toHumanText(dev.getGlobalMemory())
                  << "\n\tlocal memory\t= "     << dev.getLocalMemSize() << " " << Util::toHumanText(dev.getLocalMemSize())
                  << "\n\tavailable\t= "        << dev.isAvailable()
                  << "\n\tcompiler\t= "         << dev.isCompilerAvailable()
                  << "\n\timages\t= "           << dev.areImagesSupported()
                  << "\n\t2dsize\t= "           << dev.getImageSize()
                  << "\n\ttype\t= "             << dev.getType()
                  << "\n\tcontext\t= "          << dev.getContext()
                  << "\n\tworkGroupSize\t= "    << dev.getMaxWorkGroupSize()
                  << "\n\tworkItemSizes\t= "    << dev.getMaxWorkItemSizes()
                  << "\n\tformats\t= "          << dev.getImageFormats()
                  << "\n\textensions\t="        << dev.getExtensions()
                  << "\n}";
    return dbg.space();
}

QString ClDevice::getDeviceName(const cl_device_id id)
{
    if(!id)
        return QString();

    size_t size = 0;
    if(clGetDeviceInfo(id, CL_DEVICE_NAME, 0, 0, &size) != CL_SUCCESS)
        return QString();

    QScopedArrayPointer<char> buf(new char[size]);
    if(clGetDeviceInfo(id, CL_DEVICE_NAME, size, buf.data(), 0) != CL_SUCCESS)
        return QString();

    return QString(buf.data()).trimmed();
}

cl_uint ClDevice::getDeviceBits(const cl_device_id id)
{
    if(!id)
        return 0;

    cl_uint bits = 0;
    clGetDeviceInfo(id, CL_DEVICE_ADDRESS_BITS, sizeof(cl_uint), &bits, 0);
    return bits;
}

cl_ulong ClDevice::getDeviceGlobalMemory(const cl_device_id id)
{
    if(!id)
        return 0;

    cl_ulong size = 0;
    clGetDeviceInfo(id, CL_DEVICE_GLOBAL_MEM_SIZE, sizeof(cl_ulong), &size, 0);
    return size;
}

cl_bool ClDevice::isDeviceAvailable(const cl_device_id id)
{
    if(!id)
        return CL_FALSE;

    cl_bool available = false;
    clGetDeviceInfo(id, CL_DEVICE_AVAILABLE, sizeof(cl_bool), &available, 0);
    return available;
}

cl_bool ClDevice::isDeviceCompilerAvailable(const cl_device_id id)
{
    if(!id)
        return CL_FALSE;

    cl_bool available = false;
    clGetDeviceInfo(id, CL_DEVICE_COMPILER_AVAILABLE, sizeof(cl_bool), &available, 0);
    return available;
}

QSize ClDevice::getDeviceImageSize(const cl_device_id id)
{
    if(!id)
        return QSize();

    size_t w = 0;
    if(clGetDeviceInfo(id, CL_DEVICE_IMAGE2D_MAX_WIDTH, sizeof(size_t), &w, 0) != CL_SUCCESS)
        return QSize();

    size_t h = 0;
    if(clGetDeviceInfo(id, CL_DEVICE_IMAGE2D_MAX_HEIGHT, sizeof(size_t), &h, 0) != CL_SUCCESS)
        return QSize();

    return QSize(w, h);
}

cl_bool ClDevice::getDeviceImageSupport(const cl_device_id id)
{
    if(!id)
        return CL_FALSE;

    cl_bool available = false;
    clGetDeviceInfo(id, CL_DEVICE_IMAGE_SUPPORT, sizeof(cl_bool), &available, 0);
    return available;
}

cl_device_type ClDevice::getDeviceType(const cl_device_id id)
{
    if(!id)
        return CL_DEVICE_TYPE_ALL;

    cl_device_type type = CL_DEVICE_TYPE_ALL;
    clGetDeviceInfo(id, CL_DEVICE_TYPE, sizeof(cl_device_type), &type, 0);
    return type;
}

cl_context ClDevice::createContext(const cl_device_id id)
{
    if(!id)
        return 0;

    cl_context context = clCreateContext(nullptr, 1, &id, nullptr, nullptr, nullptr);
    return context;
}

QVector<cl_image_format> ClDevice::getContextFormats(const cl_context context)
{
    QVector<cl_image_format> formats;

    if(context)
    {
        cl_uint count = 0;
        if(clGetSupportedImageFormats(context,
                                      CL_MEM_READ_WRITE,
                                      CL_MEM_OBJECT_IMAGE2D,
                                      count,
                                      nullptr,
                                      &count) == CL_SUCCESS)
        {
            formats.fill(cl_image_format(), count);
            clGetSupportedImageFormats(context,
                                       CL_MEM_READ_WRITE,
                                       CL_MEM_OBJECT_IMAGE2D,
                                       count,
                                       formats.data(),
                                       nullptr);
        }
    }

    return formats;
}

QStringList ClDevice::getDeviceExtensions(const cl_device_id id)
{
    if(!id)
        return QStringList();

    size_t size = 0;
    if(clGetDeviceInfo(id, CL_DEVICE_EXTENSIONS, 0, 0, &size) != CL_SUCCESS)
        return QStringList();

    QScopedArrayPointer<char> buf(new char[size]);
    if(clGetDeviceInfo(id, CL_DEVICE_EXTENSIONS, size, buf.data(), 0) != CL_SUCCESS)
        return QStringList();

    return QString(buf.data()).trimmed().split(" ");
}

size_t ClDevice::getDeviceMaxWorkGroupSize(const cl_device_id id)
{
    if(!id)
        return 0;

    size_t size;
    return clGetDeviceInfo(id, CL_DEVICE_MAX_WORK_GROUP_SIZE, sizeof(size_t), &size, nullptr) == CL_SUCCESS
            ? size
            : 0;
}

QVector<size_t> ClDevice::getDeviceMaxWorkItemSizes(const cl_device_id id)
{
    if(!id)
        return QVector<size_t>();

    size_t size = 0;
    if(clGetDeviceInfo(id, CL_DEVICE_MAX_WORK_ITEM_SIZES, 0, nullptr, &size) != CL_SUCCESS)
        return QVector<size_t>();

    const size_t count = size / sizeof(size_t);
    QScopedArrayPointer<size_t> sizes(new size_t[count]);
    if(clGetDeviceInfo(id, CL_DEVICE_MAX_WORK_ITEM_SIZES, size, sizes.data(), nullptr) != CL_SUCCESS)
        return QVector<size_t>();

    QVector<size_t> vec;
    for(size_t i = 0; i < count; ++i)
    {
        vec.append(sizes[i]);
    }
    return vec;
}

cl_ulong ClDevice::getDeviceLocalMemSize(const cl_device_id id)
{
    if(!id)
        return 0;

    cl_ulong size = 0;
    return clGetDeviceInfo(id, CL_DEVICE_LOCAL_MEM_SIZE, sizeof(cl_ulong), &size, nullptr) == CL_SUCCESS
            ? size
            : 0;
}

ClDevice::ClDevice(const cl_device_id id)
    : mId(id),
      mName(getDeviceName(id)),
      mBits(getDeviceBits(id)),
      mGlobalMemory(getDeviceGlobalMemory(id)),
      mIsAvailable(isDeviceAvailable(id)),
      mIsCompilerAvailable(isDeviceCompilerAvailable(id)),
      mImageSize(getDeviceImageSize(id)),
      mImageSupport(getDeviceImageSupport(id)),
      mType(getDeviceType(id)),
      mContext(createContext(id)),
      mExtensions(getDeviceExtensions(id)),
      mMaxWorkGroupSize(getDeviceMaxWorkGroupSize(id)),
      mMaxWorkItemSizes(getDeviceMaxWorkItemSizes(id)),
      mLocalMemSize(getDeviceLocalMemSize(id))
{
    mImageFormats = getContextFormats(mContext);
}

ClDevice::ClDevice(const ClDevice &device)
    : mId(0),
      mContext(0)
{
    (*this) = device;
}

ClDevice &ClDevice::operator=(const ClDevice &device)
{
    if(this != &device)
    {
        if(mContext)
        {
            clReleaseContext(mContext);
        }
        mId                     = device.mId;
        mName                   = device.mName;
        mBits                   = device.mBits;
        mGlobalMemory           = device.mGlobalMemory;
        mIsAvailable            = device.mIsAvailable;
        mIsCompilerAvailable    = device.mIsCompilerAvailable;
        mImageSize              = device.mImageSize;
        mImageSupport           = device.mImageSupport;
        mType                   = device.mType;
        mContext                = device.mContext;
        mImageFormats           = device.mImageFormats;
        mExtensions             = device.mExtensions;
        mMaxWorkGroupSize       = device.mMaxWorkGroupSize;
        mMaxWorkItemSizes       = device.mMaxWorkItemSizes;
        mLocalMemSize           = device.mLocalMemSize;
        if(mContext)
        {
            clRetainContext(mContext);
        }
    }
    return *this;
}

ClDevice::~ClDevice()
{
    if(mContext)
    {
        clReleaseContext(mContext);
    }
}

cl_device_id ClDevice::getId()const
{
    return mId;
}

QString ClDevice::getName()const
{
    return mName;
}

cl_uint ClDevice::getBits()const
{
    return mBits;
}

cl_ulong ClDevice::getGlobalMemory()const
{
    return mGlobalMemory;
}

cl_bool ClDevice::isAvailable()const
{
    return mIsAvailable;
}

cl_bool ClDevice::isCompilerAvailable()const
{
    return mIsCompilerAvailable;
}

QSize ClDevice::getImageSize()const
{
    return mImageSize;
}

cl_bool ClDevice::areImagesSupported()const
{
    return mImageSupport;
}

cl_device_type ClDevice::getType()const
{
    return mType;
}

cl_context ClDevice::getContext()const
{
    return mContext;
}

QVector<cl_image_format> ClDevice::getImageFormats()const
{
    return mImageFormats;
}

QStringList ClDevice::getExtensions()const
{
    return mExtensions;
}

size_t ClDevice::getMaxWorkGroupSize()const
{
    return mMaxWorkGroupSize;
}

QVector<size_t> ClDevice::getMaxWorkItemSizes()const
{
    return mMaxWorkItemSizes;
}

cl_ulong ClDevice::getLocalMemSize()const
{
    return mLocalMemSize;
}
