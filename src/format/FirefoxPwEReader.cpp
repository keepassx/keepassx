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

#include "FirefoxPwEReader.h"

#include <QFile>
#include <QDebug>
#include <QTextCodec>
#include <QByteArray>
#include <QXmlStreamReader>

#include "core/Database.h"
#include "core/Endian.h"
#include "core/Entry.h"
#include "core/Group.h"

// A Reader for files from this software 
// https://addons.mozilla.org/de/firefox/addon/password-exporter/
// recommended to use the base64 export to xml

FirefoxPwEReader::FirefoxPwEReader()
    : m_error(false)
{
}

bool FirefoxPwEReader::hasError()
{
    return m_error;
}

QList<Entry*> FirefoxPwEReader::parseXml(QIODevice* device) {
    QList<Entry*> entries;

    /* fill list of entries */
    m_xml.clear();
    m_xml.setDevice(device);

    bool encoded = false;
    bool acceptedVersion = false; //Version 1.1

    while ((!m_xml.atEnd()&&!m_xml.hasError()))
    {
        /* Read next element.*/
        QXmlStreamReader::TokenType token = m_xml.readNext();
        if(token == QXmlStreamReader::StartDocument)
        {
            continue;
        }
        if(token == QXmlStreamReader::StartElement) {
            /* If it's the list start, we'll check attributes to skip parsing on unsupported version.*/
            if(m_xml.name() == "entries") {

                QXmlStreamAttributes attributes = m_xml.attributes();
                encoded = (attributes.value("encrypt").compare("true", Qt::CaseInsensitive)==0);
                acceptedVersion =
                    (attributes.value("extxmlversion").compare("1.1", Qt::CaseInsensitive)==0)&&
                    (attributes.value("ext").compare("Password Exporter", Qt::CaseInsensitive)==0);

                if (encoded)
                    qDebug() << "encoded";
                else
                    qDebug() << "uncoded";

                if (!acceptedVersion) {
                    qDebug() << "not a supported Firefox Addon Password Exporter file";
                    m_xml.skipCurrentElement();
                }
                continue;
            }
            if(m_xml.name() == "entry") {
                entries.append(this->parseXmlEntry(m_xml,encoded));
            }
        }
    }
    /* Error handling. */
    if (m_xml.hasError())
    {
        qWarning() <<  QObject::tr("%1\nLine %2, column %3")
            .arg(m_xml.errorString())
            .arg(m_xml.lineNumber())
            .arg(m_xml.columnNumber());
        QString e_msg = "Error: Import incomplete - try different export option to create password file";
        qCritical() << e_msg;
        raiseError(e_msg);
    }
    /* Removes any device() or data from the reader
     * and resets its internal state to the initial state. */
    m_xml.clear();

    return entries;
}

Database* FirefoxPwEReader::readDatabase(QIODevice* device, QString password)
{
    m_error = false;
    m_errorStr.clear();

    QScopedPointer<Database> db(new Database());
    m_db = db.data();
    m_device = device;

    //TODO check csv or xml
    bool xmlNotCsv = true;


    QList<Entry*> entries;
    if (xmlNotCsv) {
        entries = parseXml(device);
    }
    //else{
        //TODO parse CSV
    //}


    Q_FOREACH (Entry* entry, entries) {
        entry->setUuid(Uuid::random());
    }

    db->rootGroup()->setName(tr("Root"));

    Q_FOREACH (Entry* entry, m_db->rootGroup()->entriesRecursive()) {
        entry->setUpdateTimeinfo(true);
    }

    return db.take();
}

Entry* FirefoxPwEReader::parseXmlEntry(QXmlStreamReader& xml, bool encoded)
{
    QScopedPointer<Entry> entry(new Entry());
    entry->setUpdateTimeinfo(false);
    entry->setGroup(m_db->rootGroup());

    TimeInfo timeInfo;

    /* Let's get the attributes for entry */
    QXmlStreamAttributes attributes = xml.attributes();
    entry->setTitle(attributes.value("host").toString());
    entry->setUrl(attributes.value("host").toString());

    QString username = attributes.value("user").toString();
    QString password = attributes.value("password").toString();

    if (encoded) {
        //decode user name and password
        QTextCodec *codec = QTextCodec::codecForName("UTF-16");
        QTextEncoder *encoderWithoutBom = codec->makeEncoder( QTextCodec::IgnoreHeader );

        QByteArray ba = encoderWithoutBom->fromUnicode(username);
        username = QByteArray::fromBase64(ba);
        ba = encoderWithoutBom->fromUnicode(password);
        password = QByteArray::fromBase64(ba);
    }
    entry->setUsername(username);
    entry->setPassword(password);
    QString note =
        "userFieldName: " + attributes.value("password").toString() + "\n" +
        "passFieldName: " + attributes.value("passFieldName").toString() + "\n" +
        "formSubmitURL: " + attributes.value("formSubmitURL").toString() + "\n" +
        "httpRealm: " + attributes.value("httpRealm").toString() + "\n";
    entry->setNotes(note);

    QDateTime dateTime = QDateTime::currentDateTime();
    timeInfo.setExpires(false);

    if (dateTime.isValid()) {
        timeInfo.setLastAccessTime(dateTime);
    }
    if (dateTime.isValid()) {
        timeInfo.setLastModificationTime(dateTime);
    }
    if (dateTime.isValid()) {
        timeInfo.setCreationTime(dateTime);
    }
    entry->setTimeInfo(timeInfo);

    return entry.take();
}

void FirefoxPwEReader::raiseError(const QString& errorMessage)
{
    m_error = true;
    m_errorStr = errorMessage;
}

QString FirefoxPwEReader::errorString()
{
    return m_errorStr;
}

