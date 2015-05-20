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

#include "DevicesModel.h"

DevicesModel::DevicesModel()
{
}

DevicesModel::~DevicesModel()
{
}

int DevicesModel::rowCount(const QModelIndex &) const
{
    return mDevices.count();
}

QVariant DevicesModel::data(const QModelIndex &index, int role) const
{
    if(!index.isValid())
        return QVariant();

    const ClDevice device = mDevices.at(index.row());
    switch(role)
    {
        case R_Text:
            return device.getName();
    }

    return QVariant();
}

void DevicesModel::setDevices(const QList<ClDevice> devices)
{
    beginRemoveRows(QModelIndex(), 0, rowCount() - 1);
    mDevices.clear();
    endRemoveRows();

    if(devices.isEmpty())
        return;

    beginInsertRows(QModelIndex(), 0, devices.count() - 1);

    mDevices = devices;
    std::sort(mDevices.begin(), mDevices.end(), [](const ClDevice &dev1, const ClDevice &dev2){
        return dev1.getType() == dev2.getType()
                ? true
                : (dev2.getType() == CL_DEVICE_TYPE_GPU
                   ? false
                   : true);
    });

    endInsertRows();
}

ClDevice DevicesModel::at(const int row)const
{
    const bool isRowCorrect = (row >= 0) && (row < mDevices.count());
    return isRowCorrect ? mDevices.at(row) : ClDevice(0);
}

QHash<int, QByteArray> DevicesModel::roleNames() const
{
    static const QHash<int, QByteArray> roles = {
        {R_Text, "text"}
    };
    return roles;
}

QList<ClDevice> DevicesModel::getDevices()const
{
    return mDevices;
}
