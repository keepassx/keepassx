/*
*  Copyright (C) 2015 Ben Boeckel <mathstuf@gmail.com>
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

#include <QDBusMetaType>

#include "Secret.h"

Secret::Secret()
{
}

Secret::Secret(const Secret& secret)
{
    *this = secret;
}

Secret::Secret(const QDBusObjectPath& session,
               const QByteArray& parameters,
               const QByteArray& value,
               const QString& content_type) :
    m_session(session),
    m_parameters(parameters),
    m_value(value),
    m_content_type(content_type)
{
}

Secret::~Secret()
{
}

Secret& Secret::operator=(const Secret& secret)
{
    if (this == &secret) {
        return *this;
    }

    m_session = secret.m_session;
    m_parameters = secret.m_parameters;
    m_value = secret.m_value;
    m_content_type = secret.m_content_type;

    return *this;
}

QDBusObjectPath Secret::getSession() const
{
    return m_session;
}

QByteArray Secret::getParameters() const
{
    return m_parameters;
}

QByteArray Secret::getValue() const
{
    return m_value;
}

QString Secret::getContentType() const
{
    return m_content_type;
}

void Secret::registerMetaType()
{
    qRegisterMetaType<Secret>("Secret");

    qDBusRegisterMetaType<Secret>();
}

QDBusArgument& operator<<(QDBusArgument& argument, const Secret& secret)
{
    argument.beginStructure();
    argument << secret.m_session;
    argument << secret.m_parameters;
    argument << secret.m_value;
    argument << secret.m_content_type;
    argument.endStructure();

    return argument;
}

const QDBusArgument& operator>>(const QDBusArgument& argument, Secret& secret)
{
    argument.beginStructure();
    argument >> secret.m_session;
    argument >> secret.m_parameters;
    argument >> secret.m_value;
    argument >> secret.m_content_type;
    argument.endStructure();

    return argument;
}
