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

#ifndef KEEPASSX_KEEPASS2READER_H
#define KEEPASSX_KEEPASS2READER_H

#include <QCoreApplication>
#include <crypto/SymmetricCipher.h>

#include "keys/CompositeKey.h"
#include "KeePass2.h"

class Database;
class QIODevice;

class KeePass2Reader
{
    Q_DECLARE_TR_FUNCTIONS(KeePass2Reader)

public:
    KeePass2Reader();
    Database* readDatabase(QIODevice* device, const CompositeKey& key, bool keepDatabase = false);
    Database* readDatabase(const QString& filename, const CompositeKey& key);
    bool hasError();
    QString errorString();
    void setSaveXml(bool save);
    QByteArray xmlData();
    QByteArray streamKey();
    KeePass2::ProtectedStreamAlgo protectedStreamAlgo() const;

private:
    void raiseError(const QString& errorMessage);

    bool readHeaderField(QIODevice *device);
    bool readInnerHeaderField(QIODevice *device);
    QVariantMap readVariantMap(QIODevice *device);

    void setCipher(const QByteArray& data);
    void setCompressionFlags(const QByteArray& data);
    void setMasterSeed(const QByteArray& data);
    void setAesTransformSeed(const QByteArray &data);
    void setAesTransformRounds(const QByteArray &data);
    void setEncryptionIV(const QByteArray& data);
    void setProtectedStreamKey(const QByteArray& data);
    void setStreamStartBytes(const QByteArray& data);
    void setInnerRandomStreamID(const QByteArray& data);

    QIODevice* m_device;
    bool m_error;
    QString m_errorStr;
    bool m_saveXml;
    QByteArray m_xmlData;
    quint32 m_version;

    Database* m_db;
    QByteArray m_masterSeed;
    QByteArray m_encryptionIV;
    QByteArray m_streamStartBytes;
    QByteArray m_protectedStreamKey;
    QList<QByteArray> m_binaryPool;
    KeePass2::ProtectedStreamAlgo m_irsAlgo;
};

#endif // KEEPASSX_KEEPASS2READER_H
