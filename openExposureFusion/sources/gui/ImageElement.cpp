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

#include "ImageElement.h"

ImageElement::ImageElement(QQuickItem *parent)
    : QQuickPaintedItem(parent)
    , mAlignment(Qt::AlignCenter)
{
    setFillColor(Qt::transparent);
}

ImageElement::~ImageElement()
{
}

QImage ImageElement::getImage()const
{
    return mMipmaps.isEmpty() ? QImage() : mMipmaps.first();
}

void ImageElement::setImage(const QImage img)
{
    mMipmaps.clear();
    if(img.isNull())
    {
    }
    else
    {
        mMipmaps.append(img);
        const int levels = logf(std::min(img.width(), img.height())) / logf(2.0);
        for(int i = 1; i < levels; ++i)
        {
            const QImage prevImg = mMipmaps.at(i - 1);
            mMipmaps.append(prevImg.scaled(prevImg.size() / 2, Qt::KeepAspectRatio, Qt::FastTransformation));
        }
    }
    updateCache();
    update();
}

void ImageElement::paint(QPainter *painter)
{
    if(mCached.isNull())
        return;

    const QSizeF curSize(width(), height());
    const QSizeF sizeDiff = curSize - mCached.size();
    const QPointF pos(mAlignment.testFlag(Qt::AlignLeft)
                      ? 0
                      : (mAlignment.testFlag(Qt::AlignRight)
                         ? sizeDiff.width()
                         : sizeDiff.width() / 2),
                      mAlignment.testFlag(Qt::AlignTop)
                      ? 0
                      : (mAlignment.testFlag(Qt::AlignBottom)
                         ? sizeDiff.height()
                         : sizeDiff.height() / 2));
    painter->drawImage(pos, mCached);
}

void ImageElement::geometryChanged(const QRectF &newGeometry, const QRectF &oldGeometry)
{
    QQuickPaintedItem::geometryChanged(newGeometry, oldGeometry);
    updateCache();
    update();
}

void ImageElement::updateCache()
{
    if(mMipmaps.isEmpty())
    {
        mCached = QImage();
    }
    else
    {
        const QSize curSize(width(), height());
        const int level = logf(std::min(curSize.width(), curSize.height())) / logf(2.0);
        const int index = qBound(0, mMipmaps.count() - level - 1, mMipmaps.count() - 1);
        mCached = mMipmaps.at(index).scaled(curSize, Qt::KeepAspectRatio, Qt::FastTransformation);
    }
}

Qt::Alignment ImageElement::getAlignment()const
{
    return mAlignment;
}

void ImageElement::setAlignment(const Qt::Alignment flags)
{
    mAlignment = flags;
    update();
}
