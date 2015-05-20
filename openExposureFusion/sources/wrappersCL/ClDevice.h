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

#ifndef CLDEVICE_H
#define CLDEVICE_H

#include "clew/clew.h"
#include <QtCore>

class ClDevice
{
public:
    static QString                  getDeviceName(const cl_device_id id);
    static cl_uint                  getDeviceBits(const cl_device_id id);
    static cl_ulong                 getDeviceGlobalMemory(const cl_device_id id);
    static cl_bool                  isDeviceAvailable(const cl_device_id id);
    static cl_bool                  isDeviceCompilerAvailable(const cl_device_id id);
    static QSize                    getDeviceImageSize(const cl_device_id id);
    static cl_bool                  getDeviceImageSupport(const cl_device_id id);
    static cl_device_type           getDeviceType(const cl_device_id id);
    static cl_context               createContext(const cl_device_id id);
    static QVector<cl_image_format> getContextFormats(const cl_context context);
    static QStringList              getDeviceExtensions(const cl_device_id id);
    static size_t                   getDeviceMaxWorkGroupSize(const cl_device_id id);
    static QVector<size_t>          getDeviceMaxWorkItemSizes(const cl_device_id id);
    static cl_ulong                 getDeviceLocalMemSize(const cl_device_id id);

    ClDevice(const cl_device_id id);
    ClDevice(const ClDevice &device);
    ClDevice &operator=(const ClDevice &device);
    ~ClDevice();

    cl_device_id                getId()const;
    QString                     getName()const;
    cl_uint                     getBits()const;
    cl_ulong                    getGlobalMemory()const;
    cl_bool                     isAvailable()const;
    cl_bool                     isCompilerAvailable()const;
    QSize                       getImageSize()const;
    cl_bool                     areImagesSupported()const;
    cl_device_type              getType()const;
    cl_context                  getContext()const;
    QVector<cl_image_format>    getImageFormats()const;
    QStringList                 getExtensions()const;
    size_t                      getMaxWorkGroupSize()const;
    QVector<size_t>             getMaxWorkItemSizes()const;
    cl_ulong                    getLocalMemSize()const;

private:
    cl_device_id                mId;
    QString                     mName;
    cl_uint                     mBits;
    cl_ulong                    mGlobalMemory;
    cl_bool                     mIsAvailable;
    cl_bool                     mIsCompilerAvailable;
    QSize                       mImageSize;
    cl_bool                     mImageSupport;
    cl_device_type              mType;
    cl_context                  mContext;
    QVector<cl_image_format>    mImageFormats;
    QStringList                 mExtensions;
    size_t                      mMaxWorkGroupSize;
    QVector<size_t>             mMaxWorkItemSizes;
    cl_ulong                    mLocalMemSize;
};

QDebug operator<<(QDebug dbg, const cl_image_format &format);
QDebug operator<<(QDebug dbg, const ClDevice &dev);

#endif // CLDEVICE_H
