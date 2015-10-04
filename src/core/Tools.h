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

#ifndef KEEPASSX_TOOLS_H
#define KEEPASSX_TOOLS_H

#include <QDateTime>
#include <QObject>
#include <QString>

class QIODevice;

namespace Tools {

QString humanReadableFileSize(qint64 bytes);
bool hasChild(const QObject* parent, const QObject* child);
bool readFromDevice(QIODevice* device, QByteArray& data, int size = 16384);
bool readAllFromDevice(QIODevice* device, QByteArray& data);
QString imageReaderFilter();
bool isHex(const QByteArray& ba);
void sleep(int ms);
void wait(int ms);
void disableCoreDumps();

} // namespace Tools

#endif // KEEPASSX_TOOLS_H
