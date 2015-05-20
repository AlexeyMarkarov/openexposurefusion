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

#include "FilesModel.h"
#include <QtQml>
#include <QtConcurrent/QtConcurrent>

const int kColumns = 2;

FilesModel::FilesModel(QObject *parent)
    : QAbstractTableModel(parent)
    , mIsLoaderRunning(false)
{
    connect(&mFileLoadWatcher, SIGNAL(finished()), SLOT(onFileLoadWatcherFinished()));
}

FilesModel::~FilesModel()
{
}

int FilesModel::rowCount(const QModelIndex &) const
{
    return mFiles.count();
}

int FilesModel::columnCount(const QModelIndex &) const
{
    return kColumns;
}

QVariant FilesModel::data(const QModelIndex &index, int role) const
{
    static const QHash<int, QByteArray> roles = roleNames();
    if(!index.isValid())
        return QVariant();

    const FileInfo file(mFiles.at(index.row()));
    switch (role)
    {
        case R_Thumbnail:   return file.getImage();
        case R_FilePath:    return file.getFilePath();
    }
    return QVariant();
}

QVariant FilesModel::headerData(int section, Qt::Orientation orientation, int role) const
{
    switch(role)
    {
        case Qt::DisplayRole:
        {
            switch (orientation)
            {
                case Qt::Vertical:
                    return QString::number(section + 1);

                case Qt::Horizontal:
                    switch(section)
                    {
                        case 0: return tr("Thumbnail");
                        case 1: return tr("File");
                    }
                    break;
            }
            break;
        }
    }
    return QVariant();
}

bool FilesModel::removeRows(int row, int count, const QModelIndex &parent)
{
    Q_UNUSED(parent)
    if(row < 0 || row >= mFiles.count())
        return false;

    const int maxIndex = std::min(row + count, mFiles.count());
    for(int i = row; i < maxIndex; ++i)
    {
        mFilesQueue[mFiles.at(i).getFilePath()] = false;
    }
    QMetaObject::invokeMethod(this, "processNext", Qt::QueuedConnection);
    return true;
}

Qt::ItemFlags FilesModel::flags(const QModelIndex &) const
{
    return Qt::ItemIsEnabled | Qt::ItemIsSelectable;
}

QHash<int, QByteArray> FilesModel::roleNames() const
{
    static const QHash<int, QByteArray> roles = {
        {R_Thumbnail,   "thumbnail"},
        {R_FilePath,    "filepath"}
    };
    return roles;
}

void FilesModel::add(const QStringList files)
{
    for(int i = 0; i < files.count(); ++i)
    {
        mFilesQueue[files.at(i)] = true;
    }
    QMetaObject::invokeMethod(this, "processNext", Qt::QueuedConnection);
}

void FilesModel::add(const QVariant files)
{
    const QList<QUrl> urls = files.value< QList<QUrl> >();
    QStringList localFiles;
    for(int i = 0; i < urls.count(); ++i)
    {
        const QString file = urls.at(i).toLocalFile();
        if(!file.isEmpty())
            localFiles.append(file);
    }
    add(localFiles);
}

void FilesModel::remove(const int row)
{
    removeRows(row, 1);
}

void FilesModel::remove(const QVariant rows)
{
    QList<int> rowNumbers;
    if(rows.canConvert<QJSValue>())
    {
        const QJSValue value = rows.value<QJSValue>();
        if(value.isArray())
        {
            const QVariantList values = value.toVariant().toList();
            for(int i = 0; i < values.count(); ++i)
            {
                rowNumbers.append(values.at(i).toInt());
            }
        }
    }

    for(int i = 0; i < rowNumbers.count(); ++i)
    {
        const int index = rowNumbers.at(i);
        const QString path = mFiles.at(index).getFilePath();
        mFilesQueue[path] = false;
    }
    QMetaObject::invokeMethod(this, "processNext", Qt::QueuedConnection);
}

QList<FileInfo> FilesModel::getFiles()const
{
    return mFiles;
}

bool FilesModel::isEmpty()const
{
    return rowCount() <= 0;
}

void FilesModel::processNext()
{
    if(mFilesQueue.isEmpty())
        return;
    if(mIsLoaderRunning)
        return;

    const auto iter = mFilesQueue.begin();
    const QString file = iter.key();
    const bool doAdd = iter.value();
    mFilesQueue.erase(iter);

    doAdd ? add(file) : remove(file);

    if(!mFilesQueue.isEmpty())
        QMetaObject::invokeMethod(this, "processNext", Qt::QueuedConnection);
}

void FilesModel::add(const QString path)
{
    for(int i = 0; i < mFiles.count(); ++i)
    {
        if(mFiles.at(i).getFilePath() == path)
            return;
    }

    static const auto loader = [](const QString path){ return FileInfo(path); };
    mFileLoadWatcher.setFuture(QtConcurrent::run(loader, path));
    mIsLoaderRunning = true;
}

void FilesModel::remove(const QString path)
{
    int index = -1;
    for(int i = 0; i < mFiles.count(); ++i)
    {
        if(mFiles.at(i).getFilePath() == path)
        {
            index = i;
            break;
        }
    }

    if(index < 0)
        return;

    beginRemoveRows(QModelIndex(), index, index);
    mFiles.removeAt(index);
    endRemoveRows();

    emit emptyChanged();
}

void FilesModel::onFileLoadWatcherFinished()
{
    mIsLoaderRunning = false;
    const FileInfo file = mFileLoadWatcher.result();
    if(file.isValid())
    {
        beginInsertRows(QModelIndex(), mFiles.count(), mFiles.count());
        mFiles.append(file);
        endInsertRows();
        emit emptyChanged();
    }
    QMetaObject::invokeMethod(this, "processNext", Qt::QueuedConnection);
}
