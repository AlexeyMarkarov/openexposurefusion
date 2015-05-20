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

#ifndef CLPLATFORM_H
#define CLPLATFORM_H

#include "ClDevice.h"

class ClPlatform
{
public:
    static QList<ClPlatform>    getPlatforms();
    static QString              getPlatformName(const cl_platform_id id);
    static QString              getPlatformVendor(const cl_platform_id id);
    static QString              getPlatformProfile(const cl_platform_id id);
    static QString              getPlatformVersion(const cl_platform_id id);
    static QList<ClDevice>      getPlatformDevices(const cl_platform_id id);

    ClPlatform(const cl_platform_id id);

    cl_platform_id  getId()const;
    QString         getName()const;
    QString         getVendor()const;
    QString         getProfile()const;
    QString         getVersion()const;
    QList<ClDevice> getDevices()const;

private:
    cl_platform_id  mId;
    QString         mName;
    QString         mVendor;
    QString         mProfile;
    QString         mVersion;
    QList<ClDevice> mDevices;
};

QDebug operator<<(QDebug dbg, const ClPlatform &pl);

#endif // CLPLATFORM_H
