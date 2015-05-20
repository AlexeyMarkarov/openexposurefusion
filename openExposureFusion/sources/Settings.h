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

#ifndef SETTINGS_H
#define SETTINGS_H

#include <QtCore>

class Settings
{
public:
    enum Type
    {
        T_AutoUpdateView = 0,
        T_MeasureContrast,
        T_MeasureSaturation,
        T_MeasureExposedness,
        T_OutputFormat,
        T_OutputDir,
        T_max
    };

    static void set(const Type t, const QVariant value);
    static QVariant get(const Type t, const QVariant defaultValue = QVariant());
    static QVariant getDefault(const Type t);

private:
    class TypeInfo
    {
    public:
        TypeInfo(const QString k = QString(), const QVariant defv = QVariant())
            : key(k), defValue(defv)
        {}

        QString key;
        QVariant defValue;
    };
    static const QMap<Type, TypeInfo> sTypes;
    Settings();
    ~Settings();
};

#endif // SETTINGS_H
