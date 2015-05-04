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

#include "SecretService.h"

#include "adaptors/SecretServiceAdaptor.h"

static QDBusObjectPath pathForCollection(const Group* group)
{
    static const QString collectionPath = "/org/freedesktop/secrets/collection/";

    return QDBusObjectPath(collectionPath + group->uuid().toBase64());
}

static const QString sessionPath = "/org/freedesktop/secrets/session/";
static const QString aliasPath = "/org/freedesktop/secrets/aliases/";

SecretService::SecretService(Database* database) :
    QObject(database),
    m_database(database)
{
    connect(database, SIGNAL(groupDataChanged(Group*)), this, SLOT(groupChanged(Group*)));
    connect(database, SIGNAL(groupAboutToAdd(Group*,int)), this, SLOT(groupToAdd(Group*)));
    connect(database, SIGNAL(groupAdded()), this, SLOT(groupAdded()));
    connect(database, SIGNAL(groupAboutToRemove(Group*,int)), this, SLOT(groupToRemove(Group*)));
    connect(database, SIGNAL(groupRemoved()), this, SLOT(groupRemoved()));
}

SecretService::~SecretService()
{
}

void SecretService::registerWithPath(const QString& path)
{
    m_adaptor = new SecretServiceAdaptor(this);
    QDBusConnection::sessionBus().registerObject(path, this);
}

QList<QDBusObjectPath> SecretService::collections() const
{
    QList<QDBusObjectPath> paths;

    if (m_database) {
        Q_FOREACH (const Group* group, m_database->rootGroup()->groupsRecursive(true)) {
            paths.append(pathForCollection(group));
        }
    }

    return paths;
}

QDBusVariant SecretService::OpenSession(const QString& algorithm, const QDBusVariant& input, QDBusObjectPath& result)
{
    // TODO
}

QList<QDBusObjectPath> SecretService::Lock(const QList<QDBusObjectPath>& objects, QDBusObjectPath& prompt)
{
    // TODO
}

QList<QDBusObjectPath> SecretService::Unlock(const QList<QDBusObjectPath>& objects, QDBusObjectPath& prompt)
{
    // TODO
}

QDBusObjectPath SecretService::CreateCollection(const QVariantMap& properties, const QString& alias, QDBusObjectPath& prompt)
{
    // TODO
}

QList<QDBusObjectPath> SecretService::SearchItems(const QMap<QString, QString>& attributes, QList<QDBusObjectPath>& locked)
{
    // TODO
}

SecretsList SecretService::GetSecrets(const QList<QDBusObjectPath>& items, const QDBusObjectPath& session)
{
    // TODO
}

QDBusObjectPath SecretService::ReadAlias(const QString& name)
{
    // TODO
}

void SecretService::SetAlias(const QString& name, const QDBusObjectPath& collection)
{
    // TODO
}

void SecretService::groupChanged(Group* group)
{
    Q_EMIT CollectionChanged(pathForCollection(group));
}

void SecretService::groupToAdd(Group* group)
{
    m_groupToAdd = group;
}

void SecretService::groupAdded()
{
    if (!m_groupToAdd) {
        return;
    }

    QDBusObjectPath path = pathForCollection(m_groupToAdd);
    m_groupToAdd = 0;

    Q_EMIT CollectionCreated(path);
}

void SecretService::groupToRemove(Group* group)
{
    m_groupToRemove = group;
}

void SecretService::groupRemoved()
{
    if (!m_groupToRemove) {
        return;
    }

    QDBusObjectPath path = pathForCollection(m_groupToRemove);
    m_groupToRemove = 0;

    Q_EMIT CollectionDeleted(path);
}
