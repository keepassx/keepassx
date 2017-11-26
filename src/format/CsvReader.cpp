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

#include "CsvReader.h"

#include <QFile>

#include "core/Database.h"
#include "core/Entry.h"
#include "core/Group.h"
#include "keys/CompositeKey.h"

static const char sCsvSeparator(',');

namespace
{
    enum Field
    {
        IgnoredField,
        GroupField,
        TitleField,
        UsernameField,
        PasswordField,
        UrlField,
        NotesField
    };

    enum Flags
    {
        EscapedFlag  = (1 << 0),
        SwapFlag     = (1 << 1),
        NewLineFlag  = (1 << 2),

        EOFFlag      = (1 << 7)
    };
}

class CsvProcessor
{
    QByteArray m_data;
    QIODevice *m_device;
    int m_column;
    int m_state;
public:
    CsvProcessor(QIODevice *device);

    bool next();

    const QByteArray& data() const
        { return m_data; }

    int column() const
        { return m_column; }

    bool newLine() const
        { return (m_state & NewLineFlag); }

    bool atEnd() const
        { return (m_state & EOFFlag); }
};

CsvProcessor::CsvProcessor(QIODevice *device)
    : m_device(device)
{
    Q_ASSERT(m_device && m_device->isOpen() && m_device->isReadable());
}

bool CsvProcessor::next()
{
    char chr;

    /* clear buffer */
    if (m_state & SwapFlag) {
        ++m_column;
        m_data.clear();
        m_state ^= SwapFlag;
    }

    /* reset column index */
    if (m_state & NewLineFlag) {
        m_column = 0;
        m_state ^= NewLineFlag;
    }

    while (0 == (m_state & SwapFlag) && !m_device->atEnd()) {
        if (!m_device->getChar(&chr))
            return false;

        switch (chr) {
        case sCsvSeparator:
            if (m_state & EscapedFlag)
                m_data.append(chr);
            else
                m_state |= SwapFlag;
            break;

        case '"':
            if (m_state & EscapedFlag) {
                char peek;

                /* check for double escape characters */
                if (m_device->getChar(&peek)) {
                    if ('"' == peek) {
                        m_data.append(chr);
                        break;
                    }
                    else m_device->ungetChar(peek);
                }
            }

            m_state ^= EscapedFlag;
            break;

        case '\r': break; /* ignored */
        case '\n':
            if (m_state & EscapedFlag)
                m_data.append(chr);
            else
                m_state |= NewLineFlag|SwapFlag;
            break;

        default:
            m_data.append(chr);
            break;
        }
    }

    if (m_device->atEnd()) {
        if (!m_data.isEmpty())
            m_state |= NewLineFlag;
        m_state |= EOFFlag;
        return (m_state & NewLineFlag);
    }

    return true;
}

CsvReader::CsvReader()
{
}

void CsvReader::readDatabase(QIODevice* device, Database* db)
{
    CsvProcessor processor(device);

    /* popuplate headers */
    while (processor.next()) {
        const QString str = QString::fromUtf8(processor.data());

        if ((0 == (str.compare("Group", Qt::CaseInsensitive))) ||
            (0 == str.compare("grouping", Qt::CaseInsensitive)))
            m_columns.append(GroupField);
        else if ((0 == str.compare("Title", Qt::CaseInsensitive)) ||
                 (0 == str.compare("name", Qt::CaseInsensitive)))
            m_columns.append(TitleField);
        else if (0 == str.compare("Username", Qt::CaseInsensitive))
            m_columns.append(UsernameField);
        else if (0 == str.compare("Password", Qt::CaseInsensitive))
            m_columns.append(PasswordField);
        else if (0 == str.compare("URL", Qt::CaseInsensitive))
            m_columns.append(UrlField);
        else if ((0 == str.compare("Notes", Qt::CaseInsensitive)) ||
                 (0 == str.compare("extra", Qt::CaseInsensitive)))
            m_columns.append(NotesField);
        else m_columns.append(IgnoredField);

        if (processor.newLine())
            break;
    }

    const int columnCount = m_columns.count();

    /* create base group */
    Group* base = new Group();
    base->setName("CSV");
    base->setParent(db->rootGroup());
    base->setUuid(Uuid::random());

    Entry* entry = nullptr;

    /* process fields */
    while (processor.next()) {
        /* create new entry if needed */
        if (!entry) {
            entry = new Entry();
            entry->setGroup(base);
            entry->setUuid(Uuid::random());
        }

        if (processor.column() >= columnCount) {
            m_error = "Column mismatch.";
            delete entry;
            return;
        }

        switch (m_columns.at(processor.column())) {
        case GroupField:
            if (!processor.data().isEmpty()) {
                QByteArray groupPath = processor.data();
                groupPath.replace('\\', '/');

                Group* group = base;

                const QList<QByteArray> groupNames = groupPath.split('/');
                Q_FOREACH(const QByteArray &groupName, groupNames) {
                    Group* pgroup = group;

                    Q_FOREACH(Group* child, group->children()) {
                        if (0 == child->name().compare(groupName, Qt::CaseInsensitive)) {
                            group = child;
                            break;
                        }
                    }

                    if (group == pgroup) {
                        group = new Group();
                        group->setName(groupName);
                        group->setParent(pgroup);
                        group->setUuid(Uuid::random());
                    }
                }

                if (group != base)
                    entry->setGroup(group);
            }
            break;
        case NotesField:
            entry->setNotes(processor.data());
            break;
        case PasswordField:
            entry->setPassword(processor.data());
            break;
        case TitleField:
            entry->setTitle(processor.data());
            break;
        case UrlField:
            entry->setUrl(processor.data());
            break;
        case UsernameField:
            entry->setUsername(processor.data());
            break;
        case IgnoredField: break; /* ignored */
        }

        if (processor.newLine())
            entry = nullptr;
    }

    if (!processor.atEnd())
        m_error = "Failed to parse CSV file completely.";
}

bool CsvReader::hasError() const
{
    return (!m_error.isEmpty());
}

QString CsvReader::errorString() const
{
    return m_error;
}

