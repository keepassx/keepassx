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

#ifndef KEEPASSX_SECRETSERVICE_H
#define KEEPASSX_SECRETSERVICE_H

#include <QDBusObjectPath>
#include <QList>
#include <QObject>
#include <QString>
#include <QVariantMap>

#include "core/Database.h"
#include "core/Group.h"

#include "Secret.h"

class SecretServiceAdaptor;

class SecretService : public QObject
{
    Q_OBJECT

public:
    SecretService(Database* database);
    ~SecretService();

    void registerWithPath(const QString& path);

public:
    Q_PROPERTY(QList<QDBusObjectPath> Collections READ collections)
    QList<QDBusObjectPath> collections() const;

public Q_SLOTS:
    QDBusVariant OpenSession(const QString& algorithm, const QDBusVariant& input, QDBusObjectPath& result);

    QList<QDBusObjectPath> Lock(const QList<QDBusObjectPath>& objects, QDBusObjectPath& prompt);
    QList<QDBusObjectPath> Unlock(const QList<QDBusObjectPath>& objects, QDBusObjectPath& prompt);

    QDBusObjectPath CreateCollection(const QVariantMap& properties, const QString& alias, QDBusObjectPath& prompt);

    QList<QDBusObjectPath> SearchItems(const QMap<QString, QString>& attributes, QList<QDBusObjectPath>& locked);
    SecretsList GetSecrets(const QList<QDBusObjectPath>& items, const QDBusObjectPath& session);

    QDBusObjectPath ReadAlias(const QString& name);
    void SetAlias(const QString& name, const QDBusObjectPath& collection);

Q_SIGNALS:
    void CollectionChanged(const QDBusObjectPath& collection);
    void CollectionCreated(const QDBusObjectPath& collection);
    void CollectionDeleted(const QDBusObjectPath& collection);

private Q_SLOTS:
    void groupChanged(Group* group);

    void groupToAdd(Group* group);
    void groupAdded();

    void groupToRemove(Group* group);
    void groupRemoved();

private:
    SecretServiceAdaptor* m_adaptor;
    Database* m_database;

    const Group* m_groupToAdd;
    const Group* m_groupToRemove;
};

#endif // KEEPASSX_SECRETSERVICE_H
