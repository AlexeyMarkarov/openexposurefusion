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

#ifndef UTIL_H
#define UTIL_H

#include <QtCore>
#include "clew/clew.h"

class Util
{
public:
    static QString toHumanText(const qint64 bytes);
    static QString toString(const cl_int err);
    static QString toStatusString(const cl_build_status status);
    static QString toString(const cl_image_format format);
    static void release(const QVector<cl_mem> objects);
    static qint64 byteCount(const QSize size, const cl_image_format format);
    static size_t addPadding(const size_t num, const size_t pad);

    template<typename T>
    static T gcd(T a, T b);

    template<typename T>
    static T lcm(T a, T b);

private:
    Util();
    ~Util();
};

template<typename T>
T Util::gcd(T a, T b)
{
    T c;
    while(a != 0)
    {
        c = a;
        a = b % a;
        b = c;
    }
    return b;
}

template<typename T>
T Util::lcm(T a, T b)
{
    return a * b / gcd(a, b);
}

#endif // UTIL_H
