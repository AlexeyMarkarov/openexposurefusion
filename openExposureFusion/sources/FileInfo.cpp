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

#include "FileInfo.h"

FileInfo::FileInfo(const QString filePath)
    : mFilePath(filePath)
{
    if(!filePath.isEmpty())
    {
        mImg = QImage(filePath);
    }
}

FileInfo::~FileInfo()
{
}

QImage FileInfo::getImage()const
{
    return mImg;
}

QString FileInfo::getFilePath()const
{
    return mFilePath;
}

bool FileInfo::isValid() const
{
    return !mImg.isNull();
}
