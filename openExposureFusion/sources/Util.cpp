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

#include "Util.h"

QString Util::toHumanText(const qint64 bytes)
{
    const QStringList suffixes = {QObject::tr("kB"),
                                  QObject::tr("MB"),
                                  QObject::tr("GB"),
                                  QObject::tr("TB")};
    QStringListIterator iter(suffixes);
    QString units = QObject::tr("bytes");
    double tmpBytes = bytes;
    while(tmpBytes >= 1024.0 && iter.hasNext())
    {
        units = iter.next();
        tmpBytes /= 1024.0;
    }
    return QString("%1 %2").arg(tmpBytes, 0, 'f', 2).arg(units);
}

QString Util::toString(const cl_int err)
{
    static const QMap<cl_int, QString> errMap = {
        {CL_SUCCESS,                                    "CL_SUCCESS"},
        {CL_DEVICE_NOT_FOUND,                           "CL_DEVICE_NOT_FOUND"},
        {CL_DEVICE_NOT_AVAILABLE,                       "CL_DEVICE_NOT_AVAILABLE"},
        {CL_COMPILER_NOT_AVAILABLE,                     "CL_COMPILER_NOT_AVAILABLE"},
        {CL_MEM_OBJECT_ALLOCATION_FAILURE,              "CL_MEM_OBJECT_ALLOCATION_FAILURE"},
        {CL_OUT_OF_RESOURCES,                           "CL_OUT_OF_RESOURCES"},
        {CL_OUT_OF_HOST_MEMORY,                         "CL_OUT_OF_HOST_MEMORY"},
        {CL_PROFILING_INFO_NOT_AVAILABLE,               "CL_PROFILING_INFO_NOT_AVAILABLE"},
        {CL_MEM_COPY_OVERLAP,                           "CL_MEM_COPY_OVERLAP"},
        {CL_IMAGE_FORMAT_MISMATCH,                      "CL_IMAGE_FORMAT_MISMATCH"},
        {CL_IMAGE_FORMAT_NOT_SUPPORTED,                 "CL_IMAGE_FORMAT_NOT_SUPPORTED"},
        {CL_BUILD_PROGRAM_FAILURE,                      "CL_BUILD_PROGRAM_FAILURE"},
        {CL_MAP_FAILURE,                                "CL_MAP_FAILURE"},
        {CL_MISALIGNED_SUB_BUFFER_OFFSET,               "CL_MISALIGNED_SUB_BUFFER_OFFSET"},
        {CL_EXEC_STATUS_ERROR_FOR_EVENTS_IN_WAIT_LIST,  "CL_EXEC_STATUS_ERROR_FOR_EVENTS_IN_WAIT_LIST"},
        {CL_INVALID_VALUE,                              "CL_INVALID_VALUE"},
        {CL_INVALID_DEVICE_TYPE,                        "CL_INVALID_DEVICE_TYPE"},
        {CL_INVALID_PLATFORM,                           "CL_INVALID_PLATFORM"},
        {CL_INVALID_DEVICE,                             "CL_INVALID_DEVICE"},
        {CL_INVALID_CONTEXT,                            "CL_INVALID_CONTEXT"},
        {CL_INVALID_QUEUE_PROPERTIES,                   "CL_INVALID_QUEUE_PROPERTIES"},
        {CL_INVALID_COMMAND_QUEUE,                      "CL_INVALID_COMMAND_QUEUE"},
        {CL_INVALID_HOST_PTR,                           "CL_INVALID_HOST_PTR"},
        {CL_INVALID_MEM_OBJECT,                         "CL_INVALID_MEM_OBJECT"},
        {CL_INVALID_IMAGE_FORMAT_DESCRIPTOR,            "CL_INVALID_IMAGE_FORMAT_DESCRIPTOR"},
        {CL_INVALID_IMAGE_SIZE,                         "CL_INVALID_IMAGE_SIZE"},
        {CL_INVALID_SAMPLER,                            "CL_INVALID_SAMPLER"},
        {CL_INVALID_BINARY,                             "CL_INVALID_BINARY"},
        {CL_INVALID_BUILD_OPTIONS,                      "CL_INVALID_BUILD_OPTIONS"},
        {CL_INVALID_PROGRAM,                            "CL_INVALID_PROGRAM"},
        {CL_INVALID_PROGRAM_EXECUTABLE,                 "CL_INVALID_PROGRAM_EXECUTABLE"},
        {CL_INVALID_KERNEL_NAME,                        "CL_INVALID_KERNEL_NAME"},
        {CL_INVALID_KERNEL_DEFINITION,                  "CL_INVALID_KERNEL_DEFINITION"},
        {CL_INVALID_KERNEL,                             "CL_INVALID_KERNEL"},
        {CL_INVALID_ARG_INDEX,                          "CL_INVALID_ARG_INDEX"},
        {CL_INVALID_ARG_VALUE,                          "CL_INVALID_ARG_VALUE"},
        {CL_INVALID_ARG_SIZE,                           "CL_INVALID_ARG_SIZE"},
        {CL_INVALID_KERNEL_ARGS,                        "CL_INVALID_KERNEL_ARGS"},
        {CL_INVALID_WORK_DIMENSION,                     "CL_INVALID_WORK_DIMENSION"},
        {CL_INVALID_WORK_GROUP_SIZE,                    "CL_INVALID_WORK_GROUP_SIZE"},
        {CL_INVALID_WORK_ITEM_SIZE,                     "CL_INVALID_WORK_ITEM_SIZE"},
        {CL_INVALID_GLOBAL_OFFSET,                      "CL_INVALID_GLOBAL_OFFSET"},
        {CL_INVALID_EVENT_WAIT_LIST,                    "CL_INVALID_EVENT_WAIT_LIST"},
        {CL_INVALID_EVENT,                              "CL_INVALID_EVENT"},
        {CL_INVALID_OPERATION,                          "CL_INVALID_OPERATION"},
        {CL_INVALID_GL_OBJECT,                          "CL_INVALID_GL_OBJECT"},
        {CL_INVALID_BUFFER_SIZE,                        "CL_INVALID_BUFFER_SIZE"},
        {CL_INVALID_MIP_LEVEL,                          "CL_INVALID_MIP_LEVEL"},
        {CL_INVALID_GLOBAL_WORK_SIZE,                   "CL_INVALID_GLOBAL_WORK_SIZE"},
        {CL_INVALID_PROPERTY,                           "CL_INVALID_PROPERTY"}
    };
    return errMap.value(err, QObject::tr("undefined"));
}

QString Util::toStatusString(const cl_build_status status)
{
    static const QMap<cl_build_status, QString> map = {
        {CL_BUILD_ERROR,        "CL_BUILD_ERROR"},
        {CL_BUILD_IN_PROGRESS,  "CL_BUILD_IN_PROGRESS"},
        {CL_BUILD_NONE,         "CL_BUILD_NONE"},
        {CL_BUILD_SUCCESS,      "CL_BUILD_SUCCESS"}
    };
    return map.value(status, QObject::tr("undefined"));
}

void Util::release(const QVector<cl_mem> objects)
{
    for(int i = 0; i < objects.count(); ++i)
    {
        const cl_mem obj = objects.at(i);
        clReleaseMemObject(obj);
    }
}

QString Util::toString(const cl_image_format format)
{
    static const QMap<cl_channel_order, QString> channelsMap = {
        {CL_R,          "CL_R"},
        {CL_A,          "CL_A"},
        {CL_INTENSITY,  "CL_INTENSITY"},
        {CL_LUMINANCE,  "CL_LUMINANCE"},
        {CL_RG,         "CL_RG"},
        {CL_RA,         "CL_RA"},
        {CL_RGB,        "CL_RGB"},
        {CL_RGBA,       "CL_RGBA"},
        {CL_ARGB,       "CL_ARGB"},
        {CL_BGRA,       "CL_BGRA"}
    };
    static const QMap<cl_channel_type, QString> typesMap = {
        {CL_SNORM_INT8,         "CL_SNORM_INT8"},
        {CL_SNORM_INT16,        "CL_SNORM_INT16"},
        {CL_UNORM_INT8,         "CL_UNORM_INT8"},
        {CL_UNORM_INT16,        "CL_UNORM_INT16"},
        {CL_UNORM_SHORT_565,    "CL_UNORM_SHORT_565"},
        {CL_UNORM_SHORT_555,    "CL_UNORM_SHORT_555"},
        {CL_UNORM_INT_101010,   "CL_UNORM_INT_101010"},
        {CL_SIGNED_INT8,        "CL_SIGNED_INT8"},
        {CL_SIGNED_INT16,       "CL_SIGNED_INT16"},
        {CL_SIGNED_INT32,       "CL_SIGNED_INT32"},
        {CL_UNSIGNED_INT8,      "CL_UNSIGNED_INT8"},
        {CL_UNSIGNED_INT16,     "CL_UNSIGNED_INT16"},
        {CL_UNSIGNED_INT32,     "CL_UNSIGNED_INT32"},
        {CL_HALF_FLOAT,         "CL_HALF_FLOAT"},
        {CL_FLOAT,              "CL_FLOAT"}
    };
    return QString("(")
            + channelsMap.value(format.image_channel_order, QString::number(format.image_channel_order))
            + QString(", ")
            + typesMap.value(format.image_channel_data_type, QString::number(format.image_channel_data_type))
            + QString(")");
}

qint64 Util::byteCount(const QSize size, const cl_image_format format)
{
    static const QMap<cl_channel_order, qint64> channelsMap = {
        {CL_R,          1},//(r, 0.0, 0.0, 1.0)
        {CL_A,          1},//(0.0, 0.0, 0.0, a)
        {CL_INTENSITY,  1},//(I, I, I, I)
        {CL_LUMINANCE,  1},//(L, L, L, 1.0)
        {CL_RG,         2},//(r, g, 0.0, 1.0)
        {CL_RA,         2},//(r, 0.0, 0.0, a)
        {CL_RGB,        3},//(r, g, b, 1.0)
        {CL_RGBA,       4},//(r, g, b, a)
        {CL_ARGB,       4},//(r, g, b, a)
        {CL_BGRA,       4} //(r, g, b, a)
    };
    static const QMap<cl_channel_type, qint64> typesMap = {
        {CL_SNORM_INT8,         1},
        {CL_SNORM_INT16,        2},
        {CL_UNORM_INT8,         1},
        {CL_UNORM_INT16,        2},
        {CL_UNORM_SHORT_565,    2},
        {CL_SIGNED_INT8,        1},
        {CL_SIGNED_INT16,       2},
        {CL_SIGNED_INT32,       4},
        {CL_UNSIGNED_INT8,      1},
        {CL_UNSIGNED_INT16,     2},
        {CL_UNSIGNED_INT32,     4},
        {CL_HALF_FLOAT,         2},
        {CL_FLOAT,              4}
    };
    return size.width() * size.height()
            * channelsMap.value(format.image_channel_order, 0)
            * typesMap.value(format.image_channel_data_type, 0);
}

size_t Util::addPadding(const size_t num, const size_t pad)
{
    size_t result = num - num % pad;
    while(result < num)
    {
        result += pad;
    }
    return result;
}
