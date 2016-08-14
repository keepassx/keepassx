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

#include "Protocol.h"
#include <QtCore>

#include "crypto/SymmetricCipherGcrypt.h"

namespace KeepassHttpProtocol {

QHash<QString, RequestType> createStringHash()
{
    QHash<QString, RequestType> hash;
    hash.insert(STR_GET_LOGINS, GET_LOGINS);
    hash.insert(STR_GET_LOGINS_COUNT, GET_LOGINS_COUNT);
    hash.insert(STR_GET_ALL_LOGINS, GET_ALL_LOGINS);
    hash.insert(STR_SET_LOGIN, SET_LOGIN);
    hash.insert(STR_ASSOCIATE, ASSOCIATE);
    hash.insert(STR_TEST_ASSOCIATE, TEST_ASSOCIATE);
    hash.insert(STR_GENERATE_PASSWORD, GENERATE_PASSWORD);
    return hash;
}

RequestType parseRequest(const QString& str)
{
    static const QHash<QString, RequestType> REQUEST_STRINGS = createStringHash();
    return REQUEST_STRINGS.value(str, INVALID);
}

QByteArray decode64(QString s)
{
    return QByteArray::fromBase64(s.toLatin1());
}

QString encode64(QByteArray b)
{
    return QString::fromLatin1(b.toBase64());
}

QByteArray decrypt2(const QByteArray& data, SymmetricCipherGcrypt& cipher)
{
    // Ensure we get full blocks only
    if (data.length() <= 0 || data.length() % cipher.blockSize()) {
        return QByteArray();
    }

    // Decrypt
    cipher.reset();
    bool ok;
    QByteArray buffer = cipher.process(data, &ok);

    // Remove PKCS#7 padding
    buffer.chop(buffer.at(buffer.length() - 1));
    return buffer;
}

QString decrypt(const QString& data, SymmetricCipherGcrypt& cipher)
{
    return QString::fromUtf8(decrypt2(decode64(data), cipher));
}

QByteArray encrypt2(const QByteArray& data, SymmetricCipherGcrypt& cipher)
{
    // Add PKCS#7 padding
    const int blockSize = cipher.blockSize();
    const int paddingSize = blockSize - data.size() % blockSize;

    // Encrypt
    QByteArray buffer = data + QByteArray(paddingSize, paddingSize);
    cipher.reset();
    cipher.processInPlace(buffer);
    return buffer;
}

QString encrypt(const QString& data, SymmetricCipherGcrypt& cipher)
{
    return encode64(encrypt2(data.toUtf8(), cipher));
}

} // namespace KeepassHttpProtocol
