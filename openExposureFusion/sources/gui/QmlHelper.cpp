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

#include "QmlHelper.h"

QmlHelper::QmlHelper(QObject *parent)
    : QObject(parent)
{
}

QmlHelper::~QmlHelper()
{
}

void QmlHelper::aboutQt()
{
    qApp->aboutQt();
}

QString QmlHelper::toLocalFile(const QUrl url)
{
    return url.toLocalFile();
}

int QmlHelper::doubleClickInterval()
{
    return qApp->doubleClickInterval();
}

QSize QmlHelper::desktopSize()
{
    return qApp->desktop()->size();
}

QByteArray QmlHelper::readFile(const QString path)
{
    QFile file(path);
    return file.open(QFile::ReadOnly) ? file.readAll() : QByteArray();
}
