/*
 *  Copyright (C) 2012 Felix Geyer <debfx@fobos.de>
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

#include "KeePass1Reader.h"

#include <QFile>
#include <QImage>
#include <QTextCodec>

#include "core/Database.h"
#include "core/Endian.h"
#include "core/Entry.h"
#include "core/Group.h"
#include "core/Metadata.h"
#include "core/Tools.h"
#include "crypto/CryptoHash.h"
#include "format/KeePass1.h"
#include "keys/CompositeKey.h"
#include "keys/FileKey.h"
#include "keys/PasswordKey.h"
#include "streams/SymmetricCipherStream.h"

class KeePass1Key : public CompositeKey
{
public:
    virtual QByteArray rawKey() const;
    virtual void clear();
    void setPassword(const QByteArray& password);
    void setKeyfileData(const QByteArray& keyfileData);

private:
    QByteArray m_password;
    QByteArray m_keyfileData;
};


KeePass1Reader::KeePass1Reader()
    : m_db(Q_NULLPTR)
    , m_tmpParent(Q_NULLPTR)
    , m_device(Q_NULLPTR)
    , m_encryptionFlags(0)
    , m_transformRounds(0)
    , m_error(false)
{
}

Database* KeePass1Reader::readDatabase(QIODevice* device, const QString& password,
                                       QIODevice* keyfileDevice)
{
    m_error = false;
    m_errorStr.clear();

    QByteArray keyfileData;
    FileKey newFileKey;

    if (keyfileDevice) {
        keyfileData = readKeyfile(keyfileDevice);

        if (keyfileData.isEmpty()) {
            raiseError(tr("Unable to read keyfile.").append("\n").append(keyfileDevice->errorString()));
            return Q_NULLPTR;
        }
        if (!keyfileDevice->seek(0)) {
            raiseError(tr("Unable to read keyfile.").append("\n").append(keyfileDevice->errorString()));
            return Q_NULLPTR;
        }

        if (!newFileKey.load(keyfileDevice)) {
            raiseError(tr("Unable to read keyfile.").append("\n").append(keyfileDevice->errorString()));
            return Q_NULLPTR;
        }
    }

    QScopedPointer<Database> db(new Database());
    QScopedPointer<Group> tmpParent(new Group());
    m_db = db.data();
    m_tmpParent = tmpParent.data();
    m_device = device;

    bool ok;

    quint32 signature1 = Endian::readUInt32(m_device, KeePass1::BYTEORDER, &ok);
    if (!ok || signature1 != KeePass1::SIGNATURE_1) {
        raiseError(tr("Not a KeePass database."));
        return Q_NULLPTR;
    }

    quint32 signature2 = Endian::readUInt32(m_device, KeePass1::BYTEORDER, &ok);
    if (!ok || signature2 != KeePass1::SIGNATURE_2) {
        raiseError(tr("Not a KeePass database."));
        return Q_NULLPTR;
    }

    m_encryptionFlags = Endian::readUInt32(m_device, KeePass1::BYTEORDER, &ok);
    if (!ok || !(m_encryptionFlags & KeePass1::Rijndael || m_encryptionFlags & KeePass1::Twofish)) {
        raiseError(tr("Unsupported encryption algorithm."));
        return Q_NULLPTR;
    }

    quint32 version = Endian::readUInt32(m_device, KeePass1::BYTEORDER, &ok);
    if (!ok || (version & KeePass1::FILE_VERSION_CRITICAL_MASK)
            != (KeePass1::FILE_VERSION & KeePass1::FILE_VERSION_CRITICAL_MASK)) {
        raiseError(tr("Unsupported KeePass database version."));
        return Q_NULLPTR;
    }

    m_masterSeed = m_device->read(16);
    if (m_masterSeed.size() != 16) {
        raiseError("Unable to read master seed");
        return Q_NULLPTR;
    }

    m_encryptionIV = m_device->read(16);
    if (m_encryptionIV.size() != 16) {
        raiseError("Unable to read encryption IV");
        return Q_NULLPTR;
    }

    quint32 numGroups = Endian::readUInt32(m_device, KeePass1::BYTEORDER, &ok);
    if (!ok) {
        raiseError("Invalid number of groups");
        return Q_NULLPTR;
    }

    quint32 numEntries = Endian::readUInt32(m_device, KeePass1::BYTEORDER, &ok);
    if (!ok) {
        raiseError("Invalid number of entries");
        return Q_NULLPTR;
    }

    m_contentHashHeader = m_device->read(32);
    if (m_contentHashHeader.size() != 32) {
        raiseError("Invalid content hash size");
        return Q_NULLPTR;
    }

    m_transformSeed = m_device->read(32);
    if (m_transformSeed.size() != 32) {
        raiseError("Invalid transform seed size");
        return Q_NULLPTR;
    }

    m_transformRounds = Endian::readUInt32(m_device, KeePass1::BYTEORDER, &ok);
    if (!ok) {
        raiseError("Invalid number of transform rounds");
        return Q_NULLPTR;
    }
    if (!m_db->setTransformRounds(m_transformRounds)) {
        raiseError(tr("Unable to calculate master key"));
        return Q_NULLPTR;
    }

    qint64 contentPos = m_device->pos();

    QScopedPointer<SymmetricCipherStream> cipherStream(testKeys(password, keyfileData, contentPos));

    if (!cipherStream) {
        return Q_NULLPTR;
    }

    QList<Group*> groups;
    for (quint32 i = 0; i < numGroups; i++) {
        Group* group = readGroup(cipherStream.data());
        if (!group) {
            return Q_NULLPTR;
        }
        groups.append(group);
    }

    QList<Entry*> entries;
    for (quint32 i = 0; i < numEntries; i++) {
        Entry* entry = readEntry(cipherStream.data());
        if (!entry) {
            return Q_NULLPTR;
        }
        entries.append(entry);
    }

    if (!constructGroupTree(groups)) {
        raiseError("Unable to construct group tree");
        return Q_NULLPTR;
    }

    Q_FOREACH (Entry* entry, entries) {
        if (isMetaStream(entry)) {
            parseMetaStream(entry);

            delete entry;
        }
        else {
            quint32 groupId = m_entryGroupIds.value(entry);
            if (!m_groupIds.contains(groupId)) {
                qWarning("Orphaned entry found, assigning to root group.");
                entry->setGroup(m_db->rootGroup());
            } else {
                entry->setGroup(m_groupIds.value(groupId));
            }
            entry->setUuid(Uuid::random());
        }
    }

    db->rootGroup()->setName(tr("Root"));

    Q_FOREACH (Group* group, db->rootGroup()->children()) {
        if (group->name() == "Backup") {
            group->setSearchingEnabled(Group::Disable);
            group->setAutoTypeEnabled(Group::Disable);
        }
    }

    Q_ASSERT(m_tmpParent->children().isEmpty());
    Q_ASSERT(m_tmpParent->entries().isEmpty());

    Q_FOREACH (Group* group, groups) {
        group->setUpdateTimeinfo(true);
    }

    Q_FOREACH (Entry* entry, m_db->rootGroup()->entriesRecursive()) {
        entry->setUpdateTimeinfo(true);
    }

    CompositeKey key;
    if (!password.isEmpty()) {
        key.addKey(PasswordKey(password));
    }
    if (keyfileDevice) {
        key.addKey(newFileKey);
    }

    if (!db->setKey(key)) {
        raiseError(tr("Unable to calculate master key"));
        return Q_NULLPTR;
    }

    return db.take();
}

Database* KeePass1Reader::readDatabase(QIODevice* device, const QString& password,
                                       const QString& keyfileName)
{
    QScopedPointer<QFile> keyFile;
    if (!keyfileName.isEmpty()) {
        keyFile.reset(new QFile(keyfileName));
        if (!keyFile->open(QFile::ReadOnly)) {
            raiseError(keyFile->errorString());
            return Q_NULLPTR;
        }
    }

    QScopedPointer<Database> db(readDatabase(device, password, keyFile.data()));

    return db.take();
}

Database* KeePass1Reader::readDatabase(const QString& filename, const QString& password,
                                       const QString& keyfileName)
{
    QFile dbFile(filename);
    if (!dbFile.open(QFile::ReadOnly)) {
        raiseError(dbFile.errorString());
        return Q_NULLPTR;
    }

    Database* db = readDatabase(&dbFile, password, keyfileName);

    if (dbFile.error() != QFile::NoError) {
        raiseError(dbFile.errorString());
        return Q_NULLPTR;
    }

    return db;
}

bool KeePass1Reader::hasError()
{
    return m_error;
}

QString KeePass1Reader::errorString()
{
    return m_errorStr;
}

SymmetricCipherStream* KeePass1Reader::testKeys(const QString& password, const QByteArray& keyfileData,
                                                qint64 contentPos)
{
    QList<PasswordEncoding> encodings;
    encodings << Windows1252 << Latin1 << UTF8;

    QScopedPointer<SymmetricCipherStream> cipherStream;
    QByteArray passwordData;
    QTextCodec* codec = QTextCodec::codecForName("Windows-1252");
    QByteArray passwordDataCorrect = codec->fromUnicode(password);

    Q_FOREACH (PasswordEncoding encoding, encodings) {
        if (encoding == Windows1252) {
            passwordData = passwordDataCorrect;
        }
        else if (encoding == Latin1) {
            // KeePassX used Latin-1 encoding for passwords until version 0.3.1
            // but KeePass/Win32 uses Windows Codepage 1252.
            passwordData = password.toLatin1();

            if (passwordData == passwordDataCorrect) {
                continue;
            }
            else {
                qWarning("Testing password encoded as Latin-1.");
            }
        }
        else if (encoding == UTF8) {
            // KeePassX used UTF-8 encoding for passwords until version 0.2.2
            // but KeePass/Win32 uses Windows Codepage 1252.
            passwordData = password.toUtf8();

            if (passwordData == passwordDataCorrect) {
                continue;
            }
            else {
                qWarning("Testing password encoded as UTF-8.");
            }
        }

        QByteArray finalKey = key(passwordData, keyfileData);
        if (finalKey.isEmpty()) {
            return Q_NULLPTR;
        }
        if (m_encryptionFlags & KeePass1::Rijndael) {
            cipherStream.reset(new SymmetricCipherStream(m_device, SymmetricCipher::Aes256,
                    SymmetricCipher::Cbc, SymmetricCipher::Decrypt));
        }
        else {
            cipherStream.reset(new SymmetricCipherStream(m_device, SymmetricCipher::Twofish,
                    SymmetricCipher::Cbc, SymmetricCipher::Decrypt));
        }

        if (!cipherStream->init(finalKey, m_encryptionIV)) {
            raiseError(cipherStream->errorString());
            return Q_NULLPTR;
        }
        if (!cipherStream->open(QIODevice::ReadOnly)) {
            raiseError(cipherStream->errorString());
            return Q_NULLPTR;
        }

        bool success = verifyKey(cipherStream.data());

        cipherStream->reset();
        cipherStream->close();
        if (!m_device->seek(contentPos)) {
            QString msg = "unable to seek to content position";
            if (!m_device->errorString().isEmpty()) {
                msg.append("\n").append(m_device->errorString());
            }
            raiseError(msg);

            return Q_NULLPTR;
        }
        cipherStream->open(QIODevice::ReadOnly);

        if (success) {
            break;
        }
        else {
            cipherStream.reset();
        }
    }

    return cipherStream.take();
}

QByteArray KeePass1Reader::key(const QByteArray& password, const QByteArray& keyfileData)
{
    Q_ASSERT(!m_masterSeed.isEmpty());
    Q_ASSERT(!m_transformSeed.isEmpty());

    KeePass1Key key;
    key.setPassword(password);
    key.setKeyfileData(keyfileData);

    bool ok;
    QString errorString;
    QByteArray transformedKey = key.transform(m_transformSeed, m_transformRounds, &ok, &errorString);

    if (!ok) {
        raiseError(errorString);
        return QByteArray();
    }

    CryptoHash hash(CryptoHash::Sha256);
    hash.addData(m_masterSeed);
    hash.addData(transformedKey);
    return hash.result();
}

bool KeePass1Reader::verifyKey(SymmetricCipherStream* cipherStream)
{
    CryptoHash contentHash(CryptoHash::Sha256);
    QByteArray buffer;

    do {
        if (!Tools::readFromDevice(cipherStream, buffer)) {
            return false;
        }
        contentHash.addData(buffer);
    } while (!buffer.isEmpty());

    return contentHash.result() == m_contentHashHeader;
}

Group* KeePass1Reader::readGroup(QIODevice* cipherStream)
{
    QScopedPointer<Group> group(new Group());
    group->setUpdateTimeinfo(false);
    group->setParent(m_tmpParent);

    TimeInfo timeInfo;

    quint32 groupId = 0;
    quint32 groupLevel = 0;
    bool groupIdSet = false;
    bool groupLevelSet = false;

    bool ok;
    bool reachedEnd = false;

    do {
        quint16 fieldType = Endian::readUInt16(cipherStream, KeePass1::BYTEORDER, &ok);
        if (!ok) {
            raiseError("Invalid group field type number");
            return Q_NULLPTR;
        }

        int fieldSize = static_cast<int>(Endian::readUInt32(cipherStream, KeePass1::BYTEORDER, &ok));
        if (!ok) {
            raiseError("Invalid group field size");
            return Q_NULLPTR;
        }

        QByteArray fieldData = cipherStream->read(fieldSize);
        if (fieldData.size() != fieldSize) {
            raiseError("Read group field data doesn't match size");
            return Q_NULLPTR;
        }

        switch (fieldType) {
        case 0x0000:
            // ignore field
            break;
        case 0x0001:
            if (fieldSize != 4) {
                raiseError("Incorrect group id field size");
                return Q_NULLPTR;
            }
            groupId = Endian::bytesToUInt32(fieldData, KeePass1::BYTEORDER);
            groupIdSet = true;
            break;
        case 0x0002:
            group->setName(QString::fromUtf8(fieldData.constData()));
            break;
        case 0x0003:
        {
            if (fieldSize != 5) {
                raiseError("Incorrect group creation time field size");
                return Q_NULLPTR;
            }
            QDateTime dateTime = dateFromPackedStruct(fieldData);
            if (dateTime.isValid()) {
                timeInfo.setCreationTime(dateTime);
            }
            break;
        }
        case 0x0004:
        {
            if (fieldSize != 5) {
                raiseError("Incorrect group modification time field size");
                return Q_NULLPTR;
            }
            QDateTime dateTime = dateFromPackedStruct(fieldData);
            if (dateTime.isValid()) {
                timeInfo.setLastModificationTime(dateTime);
            }
            break;
        }
        case 0x0005:
        {
            if (fieldSize != 5) {
                raiseError("Incorrect group access time field size");
            }
            QDateTime dateTime = dateFromPackedStruct(fieldData);
            if (dateTime.isValid()) {
                timeInfo.setLastAccessTime(dateTime);
            }
            break;
        }
        case 0x0006:
        {
            if (fieldSize != 5) {
                raiseError("Incorrect group expiry time field size");
            }
            QDateTime dateTime = dateFromPackedStruct(fieldData);
            if (dateTime.isValid()) {
                timeInfo.setExpires(true);
                timeInfo.setExpiryTime(dateTime);
            }
            break;
        }
        case 0x0007:
        {
            if (fieldSize != 4) {
                raiseError("Incorrect group icon field size");
                return Q_NULLPTR;
            }
            quint32 iconNumber = Endian::bytesToUInt32(fieldData, KeePass1::BYTEORDER);
            group->setIcon(iconNumber);
            break;
        }
        case 0x0008:
        {
            if (fieldSize != 2) {
                raiseError("Incorrect group level field size");
                return Q_NULLPTR;
            }
            groupLevel = Endian::bytesToUInt16(fieldData, KeePass1::BYTEORDER);
            groupLevelSet = true;
            break;
        }
        case 0x0009:
            // flags, ignore field
            break;
        case 0xFFFF:
            reachedEnd = true;
            break;
        default:
            // invalid field
            raiseError("Invalid group field type");
            return Q_NULLPTR;
        }
    } while (!reachedEnd);

    if (!groupIdSet || !groupLevelSet) {
        raiseError("Missing group id or level");
        return Q_NULLPTR;
    }

    group->setUuid(Uuid::random());
    group->setTimeInfo(timeInfo);
    m_groupIds.insert(groupId, group.data());
    m_groupLevels.insert(group.data(), groupLevel);

    return group.take();
}

Entry* KeePass1Reader::readEntry(QIODevice* cipherStream)
{
    QScopedPointer<Entry> entry(new Entry());
    entry->setUpdateTimeinfo(false);
    entry->setGroup(m_tmpParent);

    TimeInfo timeInfo;
    QString binaryName;
    bool ok;
    bool reachedEnd = false;

    do {
        quint16 fieldType = Endian::readUInt16(cipherStream, KeePass1::BYTEORDER, &ok);
        if (!ok) {
            raiseError("Missing entry field type number");
            return Q_NULLPTR;
        }

        int fieldSize = static_cast<int>(Endian::readUInt32(cipherStream, KeePass1::BYTEORDER, &ok));
        if (!ok) {
            raiseError("Invalid entry field size");
            return Q_NULLPTR;
        }

        QByteArray fieldData = cipherStream->read(fieldSize);
        if (fieldData.size() != fieldSize) {
            raiseError("Read entry field data doesn't match size");
            return Q_NULLPTR;
        }

        switch (fieldType) {
        case 0x0000:
            // ignore field
            break;
        case 0x0001:
            if (fieldSize != 16) {
                raiseError("Invalid entry uuid field size");
                return Q_NULLPTR;
            }
            m_entryUuids.insert(fieldData, entry.data());
            break;
        case 0x0002:
        {
            if (fieldSize != 4) {
                raiseError("Invalid entry group id field size");
                return Q_NULLPTR;
            }
            quint32 groupId = Endian::bytesToUInt32(fieldData, KeePass1::BYTEORDER);
            m_entryGroupIds.insert(entry.data(), groupId);
            break;
        }
        case 0x0003:
        {
            if (fieldSize != 4) {
                raiseError("Invalid entry icon field size");
                return Q_NULLPTR;
            }
            quint32 iconNumber = Endian::bytesToUInt32(fieldData, KeePass1::BYTEORDER);
            entry->setIcon(iconNumber);
            break;
        }
        case 0x0004:
            entry->setTitle(QString::fromUtf8(fieldData.constData()));
            break;
        case 0x0005:
            entry->setUrl(QString::fromUtf8(fieldData.constData()));
            break;
        case 0x0006:
            entry->setUsername(QString::fromUtf8(fieldData.constData()));
            break;
        case 0x0007:
            entry->setPassword(QString::fromUtf8(fieldData.constData()));
            break;
        case 0x0008:
            parseNotes(QString::fromUtf8(fieldData.constData()), entry.data());
            break;
        case 0x0009:
        {
            if (fieldSize != 5) {
                raiseError("Invalid entry creation time field size");
                return Q_NULLPTR;
            }
            QDateTime dateTime = dateFromPackedStruct(fieldData);
            if (dateTime.isValid()) {
                timeInfo.setCreationTime(dateTime);
            }
            break;
        }
        case 0x000A:
        {
            if (fieldSize != 5) {
                raiseError("Invalid entry modification time field size");
                return Q_NULLPTR;
            }
            QDateTime dateTime = dateFromPackedStruct(fieldData);
            if (dateTime.isValid()) {
                timeInfo.setLastModificationTime(dateTime);
            }
            break;
        }
        case 0x000B:
        {
            if (fieldSize != 5) {
                raiseError("Invalid entry creation time field size");
                return Q_NULLPTR;
            }
            QDateTime dateTime = dateFromPackedStruct(fieldData);
            if (dateTime.isValid()) {
                timeInfo.setLastAccessTime(dateTime);
            }
            break;
        }
        case 0x000C:
        {
            if (fieldSize != 5) {
                raiseError("Invalid entry expiry time field size");
                return Q_NULLPTR;
            }
            QDateTime dateTime = dateFromPackedStruct(fieldData);
            if (dateTime.isValid()) {
                timeInfo.setExpires(true);
                timeInfo.setExpiryTime(dateTime);
            }
            break;
        }
        case 0x000D:
            binaryName = QString::fromUtf8(fieldData.constData());
            break;
        case 0x000E:
            if (fieldSize != 0) {
                entry->attachments()->set(binaryName, fieldData);
            }
            break;
        case 0xFFFF:
            reachedEnd = true;
            break;
        default:
            // invalid field
            raiseError("Invalid entry field type");
            return Q_NULLPTR;
        }
    } while (!reachedEnd);

    entry->setTimeInfo(timeInfo);

    return entry.take();
}

void KeePass1Reader::parseNotes(const QString& rawNotes, Entry* entry)
{
    QRegExp sequenceRegexp("Auto-Type(?:-(\\d+))?: (.+)", Qt::CaseInsensitive, QRegExp::RegExp2);
    QRegExp windowRegexp("Auto-Type-Window(?:-(\\d+))?: (.+)", Qt::CaseInsensitive, QRegExp::RegExp2);
    QHash<int, QString> sequences;
    QMap<int, QStringList> windows;

    QStringList notes;

    bool lastLineAutoType = false;
    Q_FOREACH (QString line, rawNotes.split("\n")) {
        line.remove("\r");

        if (sequenceRegexp.exactMatch(line)) {
            if (sequenceRegexp.cap(1).isEmpty()) {
                entry->setDefaultAutoTypeSequence(sequenceRegexp.cap(2));
            }
            else {
                sequences[sequenceRegexp.cap(1).toInt()] = sequenceRegexp.cap(2);
            }

            lastLineAutoType = true;
        }
        else if (windowRegexp.exactMatch(line)) {
            int nr;
            if (windowRegexp.cap(1).isEmpty()) {
                nr = -1; // special number that matches no other sequence
            }
            else {
                nr = windowRegexp.cap(1).toInt();
            }

            windows[nr].append(windowRegexp.cap(2));

            lastLineAutoType = true;
        }
        else  {
            // don't add empty lines following a removed auto-type line
            if (!lastLineAutoType || !line.isEmpty()) {
                notes.append(line);
            }
            lastLineAutoType = false;
        }
    }

    entry->setNotes(notes.join("\n"));

    QMapIterator<int, QStringList> i(windows);
    while (i.hasNext()) {
        i.next();

        QString sequence = sequences.value(i.key());

        Q_FOREACH (const QString& window, i.value()) {
            AutoTypeAssociations::Association assoc;
            assoc.window = window;
            assoc.sequence = sequence;
            entry->autoTypeAssociations()->add(assoc);
        }
    }
}

bool KeePass1Reader::constructGroupTree(const QList<Group*>& groups)
{
    for (int i = 0; i < groups.size(); i++) {
        quint32 level = m_groupLevels.value(groups[i]);

        if (level == 0) {
            groups[i]->setParent(m_db->rootGroup());
        }
        else {
            for (int j = (i - 1); j >= 0; j--) {
                if (m_groupLevels.value(groups[j]) < level) {
                    if ((level - m_groupLevels.value(groups[j])) != 1) {
                        return false;
                    }

                    groups[i]->setParent(groups[j]);
                    break;
                }
            }
        }

        if (groups[i]->parentGroup() == m_tmpParent) {
            return false;
        }
    }

    return true;
}

void KeePass1Reader::parseMetaStream(const Entry* entry)
{
    QByteArray data = entry->attachments()->value("bin-stream");

    if (entry->notes() == "KPX_GROUP_TREE_STATE") {
        if (!parseGroupTreeState(data)) {
            qWarning("Unable to parse group tree state metastream.");
        }
    }
    else if (entry->notes() == "KPX_CUSTOM_ICONS_4") {
        if (!parseCustomIcons4(data)) {
            qWarning("Unable to parse custom icons metastream.");
        }
    }
    else {
        qWarning("Ignoring unknown metastream \"%s\".", entry->notes().toLocal8Bit().constData());
    }
}

bool KeePass1Reader::parseGroupTreeState(const QByteArray& data)
{
    if (data.size() < 4) {
        return false;
    }

    int pos = 0;
    quint32 num = Endian::bytesToUInt32(data.mid(pos, 4), KeePass1::BYTEORDER);
    pos += 4;

    if (static_cast<quint32>(data.size() - 4) != (num * 5)) {
        return false;
    }

    for (quint32 i = 0; i < num; i++) {
        quint32 groupId = Endian::bytesToUInt32(data.mid(pos, 4), KeePass1::BYTEORDER);
        pos += 4;

        bool expanded = data.at(pos);
        pos += 1;

        if (m_groupIds.contains(groupId)) {
            m_groupIds[groupId]->setExpanded(expanded);
        }
    }

    return true;
}

bool KeePass1Reader::parseCustomIcons4(const QByteArray& data)
{
    if (data.size() < 12) {
        return false;
    }

    int pos = 0;

    quint32 numIcons = Endian::bytesToUInt32(data.mid(pos, 4), KeePass1::BYTEORDER);
    pos += 4;

    quint32 numEntries = Endian::bytesToUInt32(data.mid(pos, 4), KeePass1::BYTEORDER);
    pos += 4;

    quint32 numGroups = Endian::bytesToUInt32(data.mid(pos, 4), KeePass1::BYTEORDER);
    pos += 4;

    QList<Uuid> iconUuids;

    for (quint32 i = 0; i < numIcons; i++) {
        if (data.size() < (pos + 4)) {
            return false;
        }
        quint32 iconSize = Endian::bytesToUInt32(data.mid(pos, 4), KeePass1::BYTEORDER);
        pos += 4;

        if (static_cast<quint32>(data.size()) < (pos + iconSize)) {
            return false;
        }
        QImage icon = QImage::fromData(data.mid(pos, iconSize));
        pos += iconSize;

        if (icon.width() != 16 || icon.height() != 16) {
            icon = icon.scaled(16, 16);
        }

        Uuid uuid = Uuid::random();
        iconUuids.append(uuid);
        m_db->metadata()->addCustomIcon(uuid, icon);
    }

    if (static_cast<quint32>(data.size()) < (pos + numEntries * 20)) {
        return false;
    }

    for (quint32 i = 0; i < numEntries; i++) {
        QByteArray entryUuid = data.mid(pos, 16);
        pos += 16;

        int iconId = Endian::bytesToUInt32(data.mid(pos, 4), KeePass1::BYTEORDER);
        pos += 4;

        if (m_entryUuids.contains(entryUuid) && (iconId < iconUuids.size())) {
            m_entryUuids[entryUuid]->setIcon(iconUuids[iconId]);
        }
    }

    if (static_cast<quint32>(data.size()) < (pos + numGroups * 8)) {
        return false;
    }

    for (quint32 i = 0; i < numGroups; i++) {
        quint32 groupId = Endian::bytesToUInt32(data.mid(pos, 4), KeePass1::BYTEORDER);
        pos += 4;

        int iconId = Endian::bytesToUInt32(data.mid(pos, 4), KeePass1::BYTEORDER);
        pos += 4;

        if (m_groupIds.contains(groupId) && (iconId < iconUuids.size())) {
            m_groupIds[groupId]->setIcon(iconUuids[iconId]);
        }
    }

    return true;
}

void KeePass1Reader::raiseError(const QString& errorMessage)
{
    m_error = true;
    m_errorStr = errorMessage;
}

QDateTime KeePass1Reader::dateFromPackedStruct(const QByteArray& data)
{
    Q_ASSERT(data.size() == 5);

    quint32 dw1 = static_cast<uchar>(data.at(0));
    quint32 dw2 = static_cast<uchar>(data.at(1));
    quint32 dw3 = static_cast<uchar>(data.at(2));
    quint32 dw4 = static_cast<uchar>(data.at(3));
    quint32 dw5 = static_cast<uchar>(data.at(4));

    int y = (dw1 << 6) | (dw2 >> 2);
    int mon = ((dw2 & 0x00000003) << 2) | (dw3 >> 6);
    int d = (dw3 >> 1) & 0x0000001F;
    int h = ((dw3 & 0x00000001) << 4) | (dw4 >> 4);
    int min = ((dw4 & 0x0000000F) << 2) | (dw5 >> 6);
    int s = dw5 & 0x0000003F;

    QDateTime dateTime = QDateTime(QDate(y, mon, d), QTime(h, min, s), Qt::UTC);

    // check for the special "never" datetime
    if (dateTime == QDateTime(QDate(2999, 12, 28), QTime(23, 59, 59), Qt::UTC)) {
        return QDateTime();
    }
    else {
        return dateTime;
    }
}

bool KeePass1Reader::isMetaStream(const Entry* entry)
{
    return entry->attachments()->keys().contains("bin-stream")
            && !entry->notes().isEmpty()
            && entry->title() == "Meta-Info"
            && entry->username() == "SYSTEM"
            && entry->url() == "$"
            && entry->iconNumber() == 0;
}

QByteArray KeePass1Reader::readKeyfile(QIODevice* device)
{
    if (device->size() == 0) {
        return QByteArray();
    }

    if (device->size() == 32) {
        QByteArray data = device->read(32);
        if (data.size() != 32) {
            return QByteArray();
        }

        return data;
    }

    if (device->size() == 64) {
        QByteArray data = device->read(64);

        if (data.size() != 64) {
            return QByteArray();
        }

        if (Tools::isHex(data)) {
            return QByteArray::fromHex(data);
        }
        else {
            device->seek(0);
        }
    }

    CryptoHash cryptoHash(CryptoHash::Sha256);
    QByteArray buffer;

    do {
        if (!Tools::readFromDevice(device, buffer)) {
            return QByteArray();
        }
        cryptoHash.addData(buffer);
    } while (!buffer.isEmpty());

    return cryptoHash.result();
}


QByteArray KeePass1Key::rawKey() const
{
    if (m_keyfileData.isEmpty()) {
        return CryptoHash::hash(m_password, CryptoHash::Sha256);
    }
    else if (m_password.isEmpty()) {
        return m_keyfileData;
    }
    else {
        CryptoHash keyHash(CryptoHash::Sha256);
        keyHash.addData(CryptoHash::hash(m_password, CryptoHash::Sha256));
        keyHash.addData(m_keyfileData);
        return keyHash.result();
    }
}

void KeePass1Key::clear()
{
    CompositeKey::clear();

    m_password.clear();
    m_keyfileData.clear();
}

void KeePass1Key::setPassword(const QByteArray& password)
{
    m_password = password;
}

void KeePass1Key::setKeyfileData(const QByteArray& keyfileData)
{
    m_keyfileData = keyfileData;
}
