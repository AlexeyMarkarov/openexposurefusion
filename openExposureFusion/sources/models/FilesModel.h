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

#ifndef FILESMODEL_H
#define FILESMODEL_H

#include "FileInfo.h"

class FilesModel : public QAbstractTableModel
{
    Q_OBJECT
    Q_PROPERTY(bool empty READ isEmpty NOTIFY emptyChanged)

public:
    enum Roles
    {
        R_Thumbnail = Qt::UserRole,
        R_FilePath,
        R_max
    };

    FilesModel(QObject *parent = 0);
    ~FilesModel();

    virtual int rowCount(const QModelIndex &parent = QModelIndex()) const;
    virtual int columnCount(const QModelIndex &parent = QModelIndex()) const;
    virtual QVariant data(const QModelIndex &index, int role) const;
    virtual QVariant headerData(int section, Qt::Orientation orientation, int role) const;
    virtual bool removeRows(int row, int count, const QModelIndex &parent = QModelIndex());
    virtual Qt::ItemFlags flags(const QModelIndex &index) const;
    virtual QHash<int, QByteArray> roleNames() const;

    QList<FileInfo> getFiles()const;
    bool isEmpty()const;

public slots:
    void add(const QStringList files);
    void add(const QVariant files);
    void remove(const int row);
    void remove(const QVariant rows);

signals:
    void emptyChanged();

private slots:
    void processNext();
    void onFileLoadWatcherFinished();

private:
    QList<FileInfo> mFiles;
    QMap<QString, bool/*true = add; false = remove*/> mFilesQueue;
    QFutureWatcher<FileInfo> mFileLoadWatcher;
    bool mIsLoaderRunning;

    void add(const QString path);
    void remove(const QString path);
};

#endif // FILESMODEL_H
