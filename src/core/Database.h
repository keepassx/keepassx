/*
 *  Copyright (C) 2010 Felix Geyer <debfx@fobos.de>
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

#ifndef KEEPASSX_DATABASE_H
#define KEEPASSX_DATABASE_H

#include <QDateTime>
#include <QHash>

#include "core/Uuid.h"
#include "keys/CompositeKey.h"

class Entry;
class Group;
class Metadata;
class QTimer;

struct DeletedObject
{
    Uuid uuid;
    QDateTime deletionTime;
};

Q_DECLARE_TYPEINFO(DeletedObject, Q_MOVABLE_TYPE);

class Database : public QObject
{
    Q_OBJECT

public:
    enum CompressionAlgorithm
    {
        CompressionNone = 0,
        CompressionGZip = 1
    };
    static const quint32 CompressionAlgorithmMax = CompressionGZip;

    struct DatabaseData
    {
        Uuid cipher;
        CompressionAlgorithm compressionAlgo;
        QByteArray transformSeed;
        quint64 transformRounds;
        QByteArray transformedMasterKey;
        CompositeKey key;
        bool hasKey;
    };

    Database();
    ~Database();
    Group* rootGroup();
    const Group* rootGroup() const;

    /**
     * Sets group as the root group and takes ownership of it.
     * Warning: Be careful when calling this method as it doesn't
     *          emit any notifications so e.g. models aren't updated.
     *          The caller is responsible for cleaning up the previous
                root group.
     */
    void setRootGroup(Group* group);

    Metadata* metadata();
    const Metadata* metadata() const;
    Entry* resolveEntry(const Uuid& uuid);
    Group* resolveGroup(const Uuid& uuid);
    QList<DeletedObject> deletedObjects();
    void addDeletedObject(const DeletedObject& delObj);
    void addDeletedObject(const Uuid& uuid);

    Uuid cipher() const;
    Database::CompressionAlgorithm compressionAlgo() const;
    QByteArray transformSeed() const;
    quint64 transformRounds() const;
    QByteArray transformedMasterKey() const;

    void setCipher(const Uuid& cipher);
    void setCompressionAlgo(Database::CompressionAlgorithm algo);
    bool setTransformRounds(quint64 rounds);
    bool setKey(const CompositeKey& key, const QByteArray& transformSeed,
                bool updateChangedTime = true);

    /**
     * Sets the database key and generates a random transform seed.
     */
    bool setKey(const CompositeKey& key);
    bool hasKey() const;
    bool verifyKey(const CompositeKey& key) const;
    void recycleEntry(Entry* entry);
    void recycleGroup(Group* group);
    void setEmitModified(bool value);
    void copyAttributesFrom(const Database* other);

    /**
     * Returns a unique id that is only valid as long as the Database exists.
     */
    Uuid uuid();

    static Database* databaseByUuid(const Uuid& uuid);

Q_SIGNALS:
    void groupDataChanged(Group* group);
    void groupAboutToAdd(Group* group, int index);
    void groupAdded();
    void groupAboutToRemove(Group* group);
    void groupRemoved();
    void groupAboutToMove(Group* group, Group* toGroup, int index);
    void groupMoved();
    void nameTextChanged();
    void modified();
    void modifiedImmediate();

private Q_SLOTS:
    void startModifiedTimer();

private:
    Entry* recFindEntry(const Uuid& uuid, Group* group);
    Group* recFindGroup(const Uuid& uuid, Group* group);

    void createRecycleBin();

    Metadata* const m_metadata;
    Group* m_rootGroup;
    QList<DeletedObject> m_deletedObjects;
    QTimer* m_timer;
    DatabaseData m_data;
    bool m_emitModified;

    Uuid m_uuid;
    static QHash<Uuid, Database*> m_uuidMap;
};

#endif // KEEPASSX_DATABASE_H
