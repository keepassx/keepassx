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

#ifndef KEEPASSX_SECRET_H
#define KEEPASSX_SECRET_H

#include <QByteArray>
#include <QDBusArgument>
#include <QDBusObjectPath>
#include <QList>
#include <QMap>
#include <QString>

class Secret
{
public:
    Secret();
    Secret(const Secret& secret);
    Secret(const QDBusObjectPath& session,
           const QByteArray& parameters,
           const QByteArray& value,
           const QString& content_type);
    ~Secret();
    Secret& operator=(const Secret& secret);

    QDBusObjectPath getSession() const;
    QByteArray getParameters() const;
    QByteArray getValue() const;
    QString getContentType() const;

    friend QDBusArgument& operator<<(QDBusArgument& argument, const Secret& secret);
    friend const QDBusArgument& operator>>(const QDBusArgument& argument, Secret& secret);

    static void registerMetaType();

private:
    QDBusObjectPath m_session;
    QByteArray m_parameters;
    QByteArray m_value;
    QString m_content_type;
};

typedef QMap<QDBusObjectPath, Secret> SecretMap;
typedef QList<SecretMap> SecretsList;
typedef QMap<QString, QString> QStringMap;

Q_DECLARE_METATYPE(Secret)

#endif // KEEPASSX_SECRET_H
