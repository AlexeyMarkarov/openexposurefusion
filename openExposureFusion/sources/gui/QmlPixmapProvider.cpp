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

#include "QmlPixmapProvider.h"

QmlPixmapProvider::QmlPixmapProvider()
    : QQuickImageProvider(QQmlImageProviderBase::Pixmap)
{
}

QmlPixmapProvider::~QmlPixmapProvider()
{
}

QPixmap QmlPixmapProvider::requestPixmap(const QString &id, QSize *size, const QSize &requestedSize)
{
    const QStringList params = QDir::toNativeSeparators(id).split(QDir::separator(), QString::SkipEmptyParts);
    if(params.count() == 2)
    {
        if(params.first().toLower() == QString("stdicons"))
        {
            return getStdIcon(params.last(), size, requestedSize);
        }
    }
    return QQuickImageProvider::requestPixmap(id, size, requestedSize);
}

QPixmap QmlPixmapProvider::getStdIcon(const QString &id, QSize *size, const QSize &requestedSize)
{
    static const QHash<QString, QStyle::StandardPixmap> pixmaps = [](){
        QHash<QString, QStyle::StandardPixmap> map;
        const QMetaObject meta = QStyle::staticMetaObject;
        const int pixmapIndex = meta.indexOfEnumerator("StandardPixmap");
        if(pixmapIndex >= 0)
        {
            const QMetaEnum pixmapEnum = meta.enumerator(pixmapIndex);
            for(int i = 0; i < pixmapEnum.keyCount(); ++i)
            {
                map.insert(pixmapEnum.key(i), static_cast<QStyle::StandardPixmap>(pixmapEnum.value(i)));
            }
        }
        return map;
    }();

    const QStyle::StandardPixmap pixmaptype = pixmaps.value(id, QStyle::SP_CustomBase);
    const QPixmap pixmap(qApp->style()->standardPixmap(pixmaptype));
    if(size)
        *size = pixmap.size();
    const QPixmap result = requestedSize.isEmpty()
                           ? pixmap
                           : pixmap.scaled(requestedSize, Qt::KeepAspectRatio, Qt::SmoothTransformation);
    return result;
}
