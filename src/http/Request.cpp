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

#include "Request.h"
#include <QtCore>

#include "crypto/SymmetricCipher.h"
#include "crypto/SymmetricCipherGcrypt.h"

namespace KeepassHttpProtocol {

Request::Request()
    : m_requestType(INVALID)
    , m_cipher(SymmetricCipher::Aes256, SymmetricCipher::Cbc, SymmetricCipher::Decrypt)
{
    m_cipher.init();
}

QString Request::nonce() const
{
    return m_nonce;
}

void Request::setNonce(const QString& nonce)
{
    m_nonce = nonce;
}

QString Request::verifier() const
{
    return m_verifier;
}

void Request::setVerifier(const QString& verifier)
{
    m_verifier = verifier;
}

QString Request::id() const
{
    return m_id;
}

void Request::setId(const QString& id)
{
    m_id = id;
}

QString Request::key() const
{
    return m_key;
}

void Request::setKey(const QString& key)
{
    m_key = key;
}

QString Request::submitUrl() const
{
    return decrypt(m_submitUrl, m_cipher);
}

void Request::setSubmitUrl(const QString& submitUrl)
{
    m_submitUrl = submitUrl;
}

QString Request::url() const
{
    return decrypt(m_url, m_cipher);
}

void Request::setUrl(const QString& url)
{
    m_url = url;
}

QString Request::realm() const
{
    return decrypt(m_realm, m_cipher);
}

void Request::setRealm(const QString& realm)
{
    m_realm = realm;
}

QString Request::login() const
{
    return decrypt(m_login, m_cipher);
}

void Request::setLogin(const QString& login)
{
    m_login = login;
}

QString Request::uuid() const
{
    return decrypt(m_uuid, m_cipher);
}

void Request::setUuid(const QString& uuid)
{
    m_uuid = uuid;
}

QString Request::password() const
{
    return decrypt(m_password, m_cipher);
}

void Request::setPassword(const QString& password)
{
    m_password = password;
}

bool Request::sortSelection() const
{
    return m_sortSelection;
}

void Request::setSortSelection(bool sortSelection)
{
    m_sortSelection = sortSelection;
}

RequestType Request::requestType() const
{
    return parseRequest(m_requestType);
}

QString Request::requestTypeStr() const
{
    return m_requestType;
}

void Request::setRequestType(const QString& requestType)
{
    m_requestType = requestType;
}

bool Request::CheckVerifier(const QString& key) const
{
    m_cipher.setKey(decode64(key));
    m_cipher.setIv(decode64(m_nonce));
    return decrypt(m_verifier, m_cipher) == m_nonce;
}

bool Request::fromJson(QString text)
{
    QJsonDocument doc = QJsonDocument::fromJson(text.toUtf8());
    if (doc.isNull()) {
        return false;
    }

    m_requestType.clear();
    QVariantMap map = doc.object().toVariantMap();
    for (QVariantMap::iterator iter = map.begin(); iter != map.end(); ++iter) {
        setProperty(iter.key().toLatin1(), iter.value());
    }

    return requestType() != INVALID;
}

} // namespace KeepassHttpProtocol
