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

#include "DeviceInfoModel.h"
#include "Util.h"

DeviceInfoModel::DeviceInfoModel()
    : mDevice(0)
{
}

DeviceInfoModel::~DeviceInfoModel()
{
}

int DeviceInfoModel::rowCount(const QModelIndex &) const
{
    return 6 + mDevice.getImageFormats().count() + mDevice.getExtensions().count();
}

int DeviceInfoModel::columnCount(const QModelIndex &) const
{
    return R_max;
}

QVariant DeviceInfoModel::data(const QModelIndex &index, int role) const
{
    if(!index.isValid())
        return QVariant();

    const QMap<cl_device_type, QString> deviceTypes = {
        {CL_DEVICE_TYPE_ACCELERATOR,    tr("Accelerator")},
        {CL_DEVICE_TYPE_CPU,            tr("CPU")},
        {CL_DEVICE_TYPE_GPU,            tr("GPU")}
    };

    switch(index.row())
    {
        case 0:
            switch(role)
            {
                case R_Name:    return tr("Bits");
                case R_Value:   return QString::number(mDevice.getBits());
            }

        case 1:
            switch(role)
            {
                case R_Name:    return tr("Memory");
                case R_Value:   return Util::toHumanText(mDevice.getGlobalMemory());
            }

        case 2:
            switch(role)
            {
                case R_Name:    return tr("Image size");
                case R_Value:   return QString("%1 x %2")
                            .arg(mDevice.getImageSize().width())
                            .arg(mDevice.getImageSize().height());
            }

        case 3:
            switch(role)
            {
                case R_Name:    return tr("Type");
                case R_Value:   return deviceTypes.value(mDevice.getType(), QString::number(mDevice.getType()));
            }
        case 4:
            switch(role)
            {
                case R_Name:    return tr("Image formats");
                case R_Value:   return mDevice.getImageFormats().isEmpty()
                            ? QVariant()
                            : Util::toString(mDevice.getImageFormats().first());
            }
        default:
            if(mDevice.getImageFormats().count() > 1 && index.row() < (5 + mDevice.getImageFormats().count()))
            {
                switch(role)
                {
                    case R_Name:    return QVariant();
                    case R_Value:   return Util::toString(mDevice.getImageFormats().at(index.row() - 5));
                }
            }
            else
            {
                const int ind = index.row() - 5 - mDevice.getImageFormats().count();
                if(ind <= 0)
                {
                    switch(role)
                    {
                        case R_Name:    return tr("Extensions");
                        case R_Value:   return mDevice.getExtensions().isEmpty()
                                    ? QVariant()
                                    : mDevice.getExtensions().first();
                    }
                }
                else if(ind < mDevice.getExtensions().count())
                {
                    switch(role)
                    {
                        case R_Name:    return QVariant();
                        case R_Value:   return mDevice.getExtensions().at(ind);
                    }
                }
            }
    }
    return QVariant();
}

QHash<int, QByteArray> DeviceInfoModel::roleNames() const
{
    static const QHash<int, QByteArray> roles = {
        {R_Name,    "propertyname"},
        {R_Value,   "propertyvalue"}
    };
    return roles;
}

void DeviceInfoModel::setDevice(const ClDevice device)
{
    beginRemoveRows(QModelIndex(), 0, rowCount() - 2);
    endRemoveRows();
    mDevice = device;
    beginInsertRows(QModelIndex(), 0, rowCount() - 2);
    endInsertRows();
    emit dataChanged(index(0,0), index(rowCount() - 1, columnCount() - 1));
}

ClDevice DeviceInfoModel::getDevice()const
{
    return mDevice;
}
