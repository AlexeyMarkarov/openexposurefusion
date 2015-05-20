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

#ifndef DEVICEINFOMODEL_H
#define DEVICEINFOMODEL_H

#include <QtCore>
#include "wrappersCL/ClDevice.h"

class DeviceInfoModel : public QAbstractTableModel
{
public:
    enum Role {
        R_Name = 0,
        R_Value,
        R_max
    };

    DeviceInfoModel();
    ~DeviceInfoModel();

    virtual int rowCount(const QModelIndex &parent = QModelIndex()) const;
    virtual int columnCount(const QModelIndex &parent = QModelIndex()) const;
    virtual QVariant data(const QModelIndex &index, int role) const;
    virtual QHash<int, QByteArray> roleNames() const;

    void setDevice(const ClDevice device);
    ClDevice getDevice()const;

private:
    ClDevice mDevice;
};

#endif // DEVICEINFOMODEL_H
