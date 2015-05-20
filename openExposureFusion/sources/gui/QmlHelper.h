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

#ifndef QMLHELPER_H
#define QMLHELPER_H

#include <QtCore>
#include <QtGui>
#include <QtWidgets>

class QmlHelper : public QObject
{
    Q_OBJECT

public:
    QmlHelper(QObject *parent = 0);
    ~QmlHelper();

    Q_INVOKABLE void aboutQt();
    Q_INVOKABLE QString toLocalFile(const QUrl url);
    Q_INVOKABLE int doubleClickInterval();
    Q_INVOKABLE QSize desktopSize();
    Q_INVOKABLE QByteArray readFile(const QString path);
};

#endif // QMLHELPER_H
