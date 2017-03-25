/*
 *  Copyright (C) 2013 Francois Ferrand <thetypz@gmail.com>
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

#include "ResponseEntry.h"
#include <QtCore>

namespace KeepassHttpProtocol {

ResponseEntry::ResponseEntry() {}

ResponseEntry::ResponseEntry(QString name, QString login, QString password, QString uuid)
    : m_login(login)
    , m_password(password)
    , m_uuid(uuid)
    , m_name(name)
{}

ResponseEntry::ResponseEntry(const ResponseEntry& other)
    : QObject()
    , m_login(other.m_login)
    , m_password(other.m_password)
    , m_uuid(other.m_uuid)
    , m_name(other.m_name)
{}

ResponseEntry& ResponseEntry::operator=(const ResponseEntry& other) {
    m_login = other.m_login;
    m_password = other.m_password;
    m_uuid = other.m_uuid;
    m_name = other.m_name;
    return *this;
}

QString ResponseEntry::login() const
{
    return m_login;
}

QString ResponseEntry::name() const
{
    return m_name;
}

QString ResponseEntry::uuid() const
{
    return m_uuid;
}

QString ResponseEntry::password() const
{
    return m_password;
}

} // namespace KeepassHttpProtocol
