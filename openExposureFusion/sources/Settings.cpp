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

#include "Settings.h"

const QMap<Settings::Type, Settings::TypeInfo> Settings::sTypes = {
    {Settings::T_AutoUpdateView,        Settings::TypeInfo("AutoUpdateView",        false)},
    {Settings::T_MeasureContrast,       Settings::TypeInfo("MeasureContrast",       1)},
    {Settings::T_MeasureSaturation,     Settings::TypeInfo("MeasureSaturation",     1)},
    {Settings::T_MeasureExposedness,    Settings::TypeInfo("MeasureExposedness",    0)},
    {Settings::T_OutputFormat,          Settings::TypeInfo("OutputFormat",          QString())},
    {Settings::T_OutputDir,             Settings::TypeInfo("OutputDir",             QString())},
};

void Settings::set(const Type t, const QVariant value)
{
    const TypeInfo info(sTypes.value(t));
    if(info.key.isEmpty())
        return;
    QSettings(qApp->organizationName(), qApp->applicationName()).setValue(info.key, value);
}

QVariant Settings::get(const Type t, const QVariant defaultValue)
{
    const TypeInfo info(sTypes.value(t));
    return QSettings(qApp->organizationName(), qApp->applicationName()).value(info.key, defaultValue);
}

QVariant Settings::getDefault(const Type t)
{
    return sTypes.value(t).defValue;
}
