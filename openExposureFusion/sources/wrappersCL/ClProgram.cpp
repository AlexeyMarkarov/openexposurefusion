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

#include "ClProgram.h"

cl_build_status ClProgram::getProgramBuildStatus(const cl_program program, const cl_device_id device)
{
    if(!program || !device)
        return CL_BUILD_NONE;

    cl_build_status status = CL_BUILD_NONE;
    clGetProgramBuildInfo(program, device, CL_PROGRAM_BUILD_STATUS, sizeof(cl_build_status), &status, nullptr);
    return status;
}

QStringList ClProgram::getProgramBuildLog(const cl_program program, const cl_device_id device)
{
    if(!program || !device)
        return QStringList();

    size_t size = 0;
    if(clGetProgramBuildInfo(program, device, CL_PROGRAM_BUILD_LOG, 0, nullptr, &size) != CL_SUCCESS)
        return QStringList();

    QScopedArrayPointer<char> buf(new char[size]);
    if(clGetProgramBuildInfo(program, device, CL_PROGRAM_BUILD_LOG, size, buf.data(), nullptr) != CL_SUCCESS)
        return QStringList();

    const QString str(buf.data());
    return str.split("\n", QString::SkipEmptyParts);
}

ClProgram::ClProgram()
{
}

ClProgram::~ClProgram()
{
}

