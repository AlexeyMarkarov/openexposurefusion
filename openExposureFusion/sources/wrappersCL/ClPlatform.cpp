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

#include "ClPlatform.h"

QDebug operator<<(QDebug dbg, const ClPlatform &pl)
{
    dbg.nospace() << "\nClPlatform{"
                  << "\n\tid\t= "       << pl.getId()
                  << "\n\tname\t= "     << pl.getName()
                  << "\n\tvendor\t= "   << pl.getVendor()
                  << "\n\tprofile\t= "  << pl.getProfile()
                  << "\n\tversion\t= "  << pl.getVersion();
    const QList<ClDevice> devices = pl.getDevices();
    if(!devices.isEmpty())
    {
        dbg.nospace() << "\nDevices[";
        for(int i = 0; i < devices.count(); ++i)
        {
            dbg.nospace() << devices.at(i);
        }
        dbg.nospace() << "\n]";
    }

    dbg.nospace() << "\n}";
    return dbg.space();
}

QList<ClPlatform> ClPlatform::getPlatforms()
{
    QList<ClPlatform> platformList;

    cl_uint platformCount;
    switch(clGetPlatformIDs(0, 0, &platformCount))
    {
        case CL_SUCCESS:
        {
            QScopedArrayPointer<cl_platform_id> platformArray(new cl_platform_id[platformCount]);
            clGetPlatformIDs(platformCount, platformArray.data(), 0);
            for(cl_uint i = 0; i < platformCount; ++i)
            {
                platformList.append(ClPlatform(platformArray[i]));
            }
            break;
        }
    }

    return platformList;
}

QString ClPlatform::getPlatformName(const cl_platform_id id)
{
    size_t size = 0;
    if(clGetPlatformInfo(id, CL_PLATFORM_NAME, 0, 0, &size) != CL_SUCCESS)
        return QString();

    QScopedArrayPointer<char> buf(new char[size]);
    if(clGetPlatformInfo(id, CL_PLATFORM_NAME, size, buf.data(), 0) != CL_SUCCESS)
        return QString();

    return QString(buf.data()).trimmed();
}

QString ClPlatform::getPlatformVendor(const cl_platform_id id)
{
    size_t size = 0;
    if(clGetPlatformInfo(id, CL_PLATFORM_VENDOR, 0, 0, &size) != CL_SUCCESS)
        return QString();

    QScopedArrayPointer<char> buf(new char[size]);
    if(clGetPlatformInfo(id, CL_PLATFORM_VENDOR, size, buf.data(), 0) != CL_SUCCESS)
        return QString();

    return QString(buf.data()).trimmed();
}

QString ClPlatform::getPlatformProfile(const cl_platform_id id)
{
    size_t size = 0;
    if(clGetPlatformInfo(id, CL_PLATFORM_PROFILE, 0, 0, &size) != CL_SUCCESS)
        return QString();

    QScopedArrayPointer<char> buf(new char[size]);
    if(clGetPlatformInfo(id, CL_PLATFORM_PROFILE, size, buf.data(), 0) != CL_SUCCESS)
        return QString();

    return QString(buf.data()).trimmed();
}

QString ClPlatform::getPlatformVersion(const cl_platform_id id)
{
    size_t size = 0;
    if(clGetPlatformInfo(id, CL_PLATFORM_VERSION, 0, 0, &size) != CL_SUCCESS)
        return QString();

    QScopedArrayPointer<char> buf(new char[size]);
    if(clGetPlatformInfo(id, CL_PLATFORM_VERSION, size, buf.data(), 0) != CL_SUCCESS)
        return QString();

    return QString(buf.data()).trimmed();
}

QList<ClDevice> ClPlatform::getPlatformDevices(const cl_platform_id id)
{
    QList<ClDevice> devices;
    cl_uint deviceCount = 0;
    if(clGetDeviceIDs(id, CL_DEVICE_TYPE_ALL, 0, 0, &deviceCount) == CL_SUCCESS)
    {
        QScopedArrayPointer<cl_device_id> devicesArray(new cl_device_id[deviceCount]);
        if(clGetDeviceIDs(id, CL_DEVICE_TYPE_ALL, deviceCount, devicesArray.data(), 0) == CL_SUCCESS)
        {
            for(cl_uint i = 0; i < deviceCount; ++i)
            {
                devices.append(ClDevice(devicesArray[i]));
            }
        }
    }
    return devices;
}

ClPlatform::ClPlatform(const cl_platform_id id)
    : mId(id),
      mName(getPlatformName(id)),
      mVendor(getPlatformVendor(id)),
      mProfile(getPlatformProfile(id)),
      mVersion(getPlatformVersion(id)),
      mDevices(getPlatformDevices(id))
{
}

cl_platform_id ClPlatform::getId()const
{
    return mId;
}

QString ClPlatform::getName()const
{
    return mName;
}

QString ClPlatform::getVendor()const
{
    return mVendor;
}

QString ClPlatform::getProfile()const
{
    return mProfile;
}

QString ClPlatform::getVersion()const
{
    return mVersion;
}

QList<ClDevice> ClPlatform::getDevices()const
{
    return mDevices;
}
