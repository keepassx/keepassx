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

#include "KeePass2Writer.h"

#include <QBuffer>
#include <QFile>
#include <QIODevice>
#include <streams/HmacBlockStream.h>

#include "core/Database.h"
#include "core/Endian.h"
#include "crypto/CryptoHash.h"
#include "crypto/Random.h"
#include "format/KeePass2RandomStream.h"
#include "format/KeePass2XmlWriter.h"
#include "streams/HashedBlockStream.h"
#include "streams/QtIOCompressor"
#include "streams/SymmetricCipherStream.h"

#define CHECK_RETURN_FALSE(x) if (!(x)) return false;

KeePass2Writer::KeePass2Writer()
    : m_device(0)
    , m_error(false)
{
}

bool KeePass2Writer::writeDatabase(QIODevice *device, Database *db)
{
    m_error = false;
    m_errorStr.clear();

    SymmetricCipher::Algorithm algo = KeePass2::cipherUuidToAlgo(db->cipher());
    QByteArray masterSeed = randomGen()->randomArray(32);
    QByteArray encryptionIV = randomGen()->randomArray(KeePass2::getAlgoIvSize(algo));
    QByteArray protectedStreamKey = randomGen()->randomArray(64);
    QByteArray endOfHeader = "\r\n\r\n";
    QByteArray kdfParamBytes;
    serializeVariantMap(db->kdfParams(), kdfParamBytes);
    QByteArray publicCustomData = db->publicCustomData();

    CryptoHash hash(CryptoHash::Sha256);
    hash.addData(masterSeed);
    Q_ASSERT(!db->transformedMasterKey().isEmpty());
    hash.addData(db->transformedMasterKey());
    QByteArray finalKey = hash.result();

    QByteArray headerData;
    {
        QBuffer header;
        header.open(QIODevice::WriteOnly);
        m_device = &header;

        CHECK_RETURN_FALSE(writeData(Endian::int32ToBytes(KeePass2::SIGNATURE_1, KeePass2::BYTEORDER)));
        CHECK_RETURN_FALSE(writeData(Endian::int32ToBytes(KeePass2::SIGNATURE_2, KeePass2::BYTEORDER)));
        CHECK_RETURN_FALSE(writeData(Endian::int32ToBytes(KeePass2::FILE_VERSION, KeePass2::BYTEORDER)));

        CHECK_RETURN_FALSE(writeHeaderField(KeePass2::CipherID, db->cipher().toByteArray()));
        CHECK_RETURN_FALSE(writeHeaderField(KeePass2::CompressionFlags,
                                            Endian::int32ToBytes(db->compressionAlgo(),
                                                                 KeePass2::BYTEORDER)));
        CHECK_RETURN_FALSE(writeHeaderField(KeePass2::MasterSeed, masterSeed));
        CHECK_RETURN_FALSE(writeHeaderField(KeePass2::EncryptionIV, encryptionIV));
        CHECK_RETURN_FALSE(writeHeaderField(KeePass2::KdfParameters, kdfParamBytes));
        if (!publicCustomData.isEmpty()) {
            CHECK_RETURN_FALSE(writeHeaderField(KeePass2::PublicCustomData, publicCustomData));
        }
        CHECK_RETURN_FALSE(writeHeaderField(KeePass2::EndOfHeader, endOfHeader));
        header.close();
        m_device = device;
        headerData = header.data();
    }

    QByteArray headerHash = CryptoHash::hash(headerData, CryptoHash::Sha256);
    QByteArray hmacKey = KeePass2::hmacKey(masterSeed, db->transformedMasterKey());
    QByteArray headerHmac = CryptoHash::hmac(headerData, HmacBlockStream::getHmacKey(UINT64_MAX, hmacKey), CryptoHash::Sha256);
    CHECK_RETURN_FALSE(writeData(headerData));
    CHECK_RETURN_FALSE(writeData(headerHash));
    CHECK_RETURN_FALSE(writeData(headerHmac));

    HmacBlockStream hmacStream(device, hmacKey);
    if (!hmacStream.open(QIODevice::WriteOnly)) {
        raiseError(hmacStream.errorString());
        return false;
    }

    SymmetricCipherStream cipherStream(&hmacStream, algo, KeePass2::getAlgoMode(algo),
                                       SymmetricCipher::Encrypt);
    cipherStream.init(finalKey, encryptionIV);
    if (!cipherStream.open(QIODevice::WriteOnly)) {
        raiseError(cipherStream.errorString());
        return false;
    }

    QScopedPointer<QtIOCompressor> ioCompressor;
    if (db->compressionAlgo() == Database::CompressionNone) {
        m_device = &cipherStream;
    }
    else {
        ioCompressor.reset(new QtIOCompressor(&cipherStream));
        ioCompressor->setStreamFormat(QtIOCompressor::GzipFormat);
        if (!ioCompressor->open(QIODevice::WriteOnly)) {
            raiseError(ioCompressor->errorString());
            return false;
        }
        m_device = ioCompressor.data();
    }

    CHECK_RETURN_FALSE(writeInnerHeaderField(KeePass2::InnerHeaderFieldID::InnerRandomStreamID,
                                       Endian::int32ToBytes(KeePass2::ChaCha20, KeePass2::BYTEORDER)));
    CHECK_RETURN_FALSE(writeInnerHeaderField(KeePass2::InnerHeaderFieldID::InnerRandomStreamKey,
                                       protectedStreamKey));
    QHash<QByteArray, int> idMap;
    const QList<Entry*> allEntries = db->rootGroup()->entriesRecursive(true);
    int nextId = 0;

    for (Entry* entry : allEntries) {
        const QList<QString> attachmentKeys = entry->attachments()->keys();
        for (const QString& key : attachmentKeys) {
            QByteArray data = entry->attachments()->value(key);
            if (!idMap.contains(data)) {
                CHECK_RETURN_FALSE(writeBinary(data));
                idMap.insert(data, nextId++);
            }
        }
    }
    CHECK_RETURN_FALSE(writeInnerHeaderField(KeePass2::InnerHeaderFieldID::End, QByteArray()));

    KeePass2RandomStream randomStream(KeePass2::ChaCha20);
    if (!randomStream.init(protectedStreamKey)) {
        raiseError(randomStream.errorString());
        return false;
    }

    KeePass2XmlWriter xmlWriter(idMap);
    xmlWriter.writeDatabase(m_device, db, &randomStream);

    // Explicitly close/reset streams so they are flushed and we can detect
    // errors. QIODevice::close() resets errorString() etc.
    if (ioCompressor) {
        ioCompressor->close();
    }
    if (!cipherStream.reset()) {
        raiseError(cipherStream.errorString());
        return false;
    }
    if (!hmacStream.reset()) {
        raiseError(hmacStream.errorString());
        return false;
    }

    if (xmlWriter.hasError()) {
        raiseError(xmlWriter.errorString());
        return false;
    }

    return true;
}

bool KeePass2Writer::writeData(const QByteArray& data)
{
    if (m_device->write(data) != data.size()) {
        raiseError(m_device->errorString());
        return false;
    }
    else {
        return true;
    }
}

bool KeePass2Writer::writeHeaderField(KeePass2::HeaderFieldID fieldId, const QByteArray& data)
{
    Q_ASSERT(data.size() <= static_cast<qint64>(UINT32_MAX));

    QByteArray fieldIdArr;
    fieldIdArr[0] = fieldId;
    CHECK_RETURN_FALSE(writeData(fieldIdArr));
    CHECK_RETURN_FALSE(writeData(Endian::uint32ToBytes(static_cast<quint32>(data.size()), KeePass2::BYTEORDER)));
    CHECK_RETURN_FALSE(writeData(data));

    return true;
}

bool KeePass2Writer::writeInnerHeaderField(KeePass2::InnerHeaderFieldID fieldId, const QByteArray& data)
{
    Q_ASSERT(data.size() <= static_cast<qint64>(UINT32_MAX));

    QByteArray fieldIdArr;
    fieldIdArr[0] = static_cast<char>(fieldId);
    CHECK_RETURN_FALSE(writeData(fieldIdArr));
    CHECK_RETURN_FALSE(writeData(Endian::uint32ToBytes(static_cast<quint32>(data.size()), KeePass2::BYTEORDER)));
    CHECK_RETURN_FALSE(writeData(data));

    return true;
}

bool KeePass2Writer::writeBinary(const QByteArray& data)
{
    Q_ASSERT(data.size() <= static_cast<qint64>(UINT32_MAX - 1));

    QByteArray fieldIdArr;
    fieldIdArr[0] = static_cast<char>(KeePass2::InnerHeaderFieldID::Binary);
    CHECK_RETURN_FALSE(writeData(fieldIdArr));
    CHECK_RETURN_FALSE(writeData(Endian::uint32ToBytes(static_cast<quint32>(data.size() + 1), KeePass2::BYTEORDER)));
    CHECK_RETURN_FALSE(writeData(QByteArray(1, '\1')));
    CHECK_RETURN_FALSE(writeData(data));

    return true;
}

bool KeePass2Writer::serializeVariantMap(const QVariantMap& p, QByteArray& o)
{
    QBuffer buf(&o);
    buf.open(QIODevice::WriteOnly);
    CHECK_RETURN_FALSE(buf.write(Endian::uint16ToBytes(KeePass2::VARIANTMAP_VERSION, KeePass2::BYTEORDER)) == 2);

    bool ok;
    QVariantMap::key_iterator k = p.keyBegin();
    for (; k != p.keyEnd(); ++k) {
        KeePass2::VariantMapFieldType fieldType;
        QByteArray data;
        QVariant v = p.value(*k);
        switch (static_cast<QMetaType::Type>(v.type())) {
            case QMetaType::Type::Int:
                fieldType = KeePass2::VariantMapFieldType::Int32;
                data = Endian::int32ToBytes(v.toInt(&ok), KeePass2::BYTEORDER);
                CHECK_RETURN_FALSE(ok);
                break;
            case QMetaType::Type::UInt:
                fieldType = KeePass2::VariantMapFieldType::UInt32;
                data = Endian::uint32ToBytes(v.toUInt(&ok), KeePass2::BYTEORDER);
                CHECK_RETURN_FALSE(ok);
                break;
            case QMetaType::Type::LongLong:
                fieldType = KeePass2::VariantMapFieldType::Int64;
                data = Endian::int64ToBytes(v.toLongLong(&ok), KeePass2::BYTEORDER);
                CHECK_RETURN_FALSE(ok);
                break;
            case QMetaType::Type::ULongLong:
                fieldType = KeePass2::VariantMapFieldType::UInt64;
                data = Endian::uint64ToBytes(v.toULongLong(&ok), KeePass2::BYTEORDER);
                CHECK_RETURN_FALSE(ok);
                break;
            case QMetaType::Type::QString:
                fieldType = KeePass2::VariantMapFieldType::String;
                data = v.toString().toUtf8();
                break;
            case QMetaType::Type::Bool:
                fieldType = KeePass2::VariantMapFieldType::Bool;
                data = QByteArray(1, (v.toBool() ? '\1' : '\0'));
                break;
            case QMetaType::Type::QByteArray:
                fieldType = KeePass2::VariantMapFieldType::ByteArray;
                data = v.toByteArray();
                break;
            default:
                qWarning("Unknown object type %d in QVariantMap", v.type());
                return false;
        }
        QByteArray typeBytes;
        typeBytes[0] = static_cast<char>(fieldType);
        QByteArray nameBytes = k->toUtf8();
        QByteArray nameLenBytes = Endian::int32ToBytes(nameBytes.size(), KeePass2::BYTEORDER);
        QByteArray dataLenBytes = Endian::int32ToBytes(data.size(), KeePass2::BYTEORDER);

        CHECK_RETURN_FALSE(buf.write(typeBytes) == 1);
        CHECK_RETURN_FALSE(buf.write(nameLenBytes) == 4);
        CHECK_RETURN_FALSE(buf.write(nameBytes) == nameBytes.size());
        CHECK_RETURN_FALSE(buf.write(dataLenBytes) == 4);
        CHECK_RETURN_FALSE(buf.write(data) == data.size());
    }

    QByteArray endBytes;
    endBytes[0] = static_cast<char>(KeePass2::VariantMapFieldType::End);
    CHECK_RETURN_FALSE(buf.write(endBytes) == 1);
    return true;
}

void KeePass2Writer::writeDatabase(const QString& filename, Database* db)
{
    QFile file(filename);
    if (!file.open(QIODevice::WriteOnly|QIODevice::Truncate)) {
        raiseError(file.errorString());
        return;
    }
    writeDatabase(&file, db);
}

bool KeePass2Writer::hasError()
{
    return m_error;
}

QString KeePass2Writer::errorString()
{
    return m_errorStr;
}

void KeePass2Writer::raiseError(const QString& errorMessage)
{
    m_error = true;
    m_errorStr = errorMessage;
}