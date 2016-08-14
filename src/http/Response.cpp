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

#include "Response.h"
#include <QtCore>

#include "core/Global.h"
#include "crypto/Random.h"
#include "crypto/SymmetricCipher.h"
#include "crypto/SymmetricCipherGcrypt.h"

static QVariantMap qobject2qvariant(const QObject* object, const QStringList& ignoredProperties = QStringList(QString(QLatin1String("objectName"))))
{
    QVariantMap result;
    const QMetaObject* metaobject = object->metaObject();
    int count = metaobject->propertyCount();
    for (int i = 0; i < count; ++i) {
        QMetaProperty metaproperty = metaobject->property(i);
        const char* name = metaproperty.name();

        if (!ignoredProperties.contains(QLatin1String(name)) && metaproperty.isReadable()) {
            QVariant value = object->property(name);
            result[QLatin1String(name)] = value;
        }
    }
    return result;
}

namespace KeepassHttpProtocol {

Response::Response(const Request& request, QString hash)
    : m_requestType(request.requestTypeStr()), m_success(false), m_count(-1)
    , m_version(STR_VERSION), m_hash(hash)
    , m_cipher(SymmetricCipher::Aes256, SymmetricCipher::Cbc, SymmetricCipher::Encrypt)
{
    m_cipher.init();
}

void Response::setVerifier(QString key)
{
    m_cipher.setKey(decode64(key));

    // Generate new IV
    const QByteArray iv = randomGen()->randomArray(m_cipher.blockSize());
    m_cipher.setIv(iv);
    m_nonce = encode64(iv);

    // Encrypt
    m_verifier = encrypt(m_nonce, m_cipher);
}

QString Response::toJson()
{
    QVariant result = qobject2qvariant(this);

    QJsonDocument doc = QJsonDocument::fromVariant(result);
    return doc.toJson(QJsonDocument::Compact);
}

RequestType Response::requestType() const
{
    return parseRequest(m_requestType);
}

QString Response::requestTypeStr() const
{
    return m_requestType;
}

QString Response::verifier() const
{
    return m_verifier;
}

QString Response::nonce() const
{
    return m_nonce;
}

QVariant Response::count() const
{
    return m_count < 0 ? QVariant() : QVariant(m_count);
}

void Response::setCount(int count)
{
    m_count = count;
}

QVariant Response::getEntries() const
{
    if (m_count < 0 || m_entries.isEmpty()) {
        return QVariant();
    }

    QList<QVariant> res;
    res.reserve(m_entries.size());
    for (const ResponseEntry& entry : asConst(m_entries)) {
        res.append(qobject2qvariant(&entry));
    }
    return res;
}

void Response::setEntries(const QList<ResponseEntry>& entries)
{
    m_count = entries.count();

    QList<ResponseEntry> encryptedEntries;
    encryptedEntries.reserve(m_count);
    for (const ResponseEntry& entry : entries) {
        encryptedEntries << ResponseEntry(
            encrypt(entry.name(), m_cipher), encrypt(entry.login(), m_cipher),
            entry.password().isNull() ? QString()
                                      : encrypt(entry.password(), m_cipher),
            encrypt(entry.uuid(), m_cipher));
    }
    m_entries = encryptedEntries;
}

QString Response::hash() const
{
    return m_hash;
}

QString Response::version() const
{
    return m_version;
}

QString Response::id() const
{
    return m_id;
}

void Response::setId(const QString& id)
{
    m_id = id;
}

bool Response::success() const
{
    return m_success;
}

void Response::setSuccess()
{
    m_success = true;
}

QString Response::error() const
{
    return m_error;
}

void Response::setError(const QString& error)
{
    m_success = false;
    m_error = error;
}

} // namespace KeepassHttpProtocol
