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

#ifndef IMAGEELEMENT_H
#define IMAGEELEMENT_H

#include <QtCore>
#include <QtQuick>

class ImageElement : public QQuickPaintedItem
{
    Q_OBJECT
    Q_PROPERTY(QImage           img         READ getImage       WRITE setImage)
    Q_PROPERTY(Qt::Alignment    alignment   READ getAlignment   WRITE setAlignment)

public:
    ImageElement(QQuickItem *parent = 0);
    ~ImageElement();

    QImage getImage()const;
    Qt::Alignment getAlignment()const;

    virtual void paint(QPainter *painter);
    virtual void geometryChanged(const QRectF &newGeometry, const QRectF &oldGeometry);

public slots:
    void setImage(const QImage img);
    void setAlignment(const Qt::Alignment flags);

private:
    Qt::Alignment mAlignment;
    QVector<QImage> mMipmaps;
    QImage mCached;

    void updateCache();
};

#endif // IMAGEELEMENT_H
