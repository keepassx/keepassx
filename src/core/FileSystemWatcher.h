/*
 *  Copyright (C) 2012 Felix Geyer <debfx@fobos.de>
 *
 *  This program is free software: you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation, either version 2 or (at your option)
 *  version 3 of the License.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#ifndef FILE_SYSTEM_WATCHER_H
#define FILE_SYSTEM_WATCHER_H
#include <QFileSystemWatcher>
#include <QString>
#include <QObject>


class FileSystemWatcher : public QFileSystemWatcher
{
    Q_OBJECT ;

public:
    FileSystemWatcher ();
    void watchFile( const QString & );
    void stopWatching();

    virtual ~FileSystemWatcher();

private:
    QString _file;

private Q_SLOTS:
    void directoryChangedSlot ( const QString & );
    void fileChangedSlot ( const QString & );
Q_SIGNALS:
    void fileChanged();
};

#endif

