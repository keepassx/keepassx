/*
 *  Copyright (C) 2015 Florian Geyer <blueice@fobos.de>
 *  Copyright (C) 2015 Felix Geyer <debfx@fobos.de>
 *  Copyright (C) 2015 Guillermo A. Amaral B. <g@maral.me>
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

#ifndef KEEPASSX_CSVREADER_H
#define KEEPASSX_CSVREADER_H

#include <QString>
#include <QVector>

class Database;
class QIODevice;

class CsvReader
{
public:
    CsvReader();

    void readDatabase(QIODevice* device, Database* db);
    bool hasError() const;
    QString errorString() const;

private:
    QVector<int> m_columns;
    QString m_error;
};

#endif // KEEPASSX_CSVREADER_H
