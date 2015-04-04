/*
 *  Copyright (C) 2014 Karsten Hinz <k.hinz@tu-bs.de>
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

#ifndef KEEPASSX_FIREFOXPWEREADER_H
#define KEEPASSX_FIREFOXPWEREADER_H

#include <QCoreApplication>
#include <QXmlStreamReader>
#include <QDateTime>
#include <QHash>

class Database;
class Entry;
class Group;
class QIODevice;

class FirefoxPwEReader
{
    Q_DECLARE_TR_FUNCTIONS(FirefoxPwEReader)

public:
    FirefoxPwEReader();
    Database* readDatabase(QIODevice* device, QString password);
    bool hasError();
    QString errorString();

private:
    Entry* parseXmlEntry(QXmlStreamReader& xml, bool encrypted);
    QList<Entry*> parseXml(QIODevice* device);
    void raiseError(const QString& errorMessage);

    Database* m_db;
    QIODevice* m_device;
    QXmlStreamReader m_xml;

    bool m_error;
    QString m_errorStr;
};

#endif // KEEPASSX_FIREFOXPWEREADER_H
