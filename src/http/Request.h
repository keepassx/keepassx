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

#ifndef REQUEST_H
#define REQUEST_H

#include "Protocol.h"
#include <QtCore>
#include "crypto/SymmetricCipherGcrypt.h"

namespace KeepassHttpProtocol {

//TODO: use QByteArray whenever possible?

class Request : public QObject
{
    Q_OBJECT
    Q_PROPERTY(QString RequestType   READ requestTypeStr WRITE setRequestType  )
    Q_PROPERTY(bool    SortSelection READ sortSelection  WRITE setSortSelection)
    Q_PROPERTY(QString Login         READ login          WRITE setLogin        )
    Q_PROPERTY(QString Password      READ password       WRITE setPassword     )
    Q_PROPERTY(QString Uuid          READ uuid           WRITE setUuid         )
    Q_PROPERTY(QString Url           READ url            WRITE setUrl          )
    Q_PROPERTY(QString SubmitUrl     READ submitUrl      WRITE setSubmitUrl    )
    Q_PROPERTY(QString Key           READ key            WRITE setKey          )
    Q_PROPERTY(QString Id            READ id             WRITE setId           )
    Q_PROPERTY(QString Verifier      READ verifier       WRITE setVerifier     )
    Q_PROPERTY(QString Nonce         READ nonce          WRITE setNonce        )
    Q_PROPERTY(QString Realm         READ realm          WRITE setRealm        )

public:
    Request();
    bool fromJson(QString text);

    RequestType requestType() const;
    QString requestTypeStr() const;
    bool sortSelection() const;
    QString login() const;
    QString password() const;
    QString uuid() const;
    QString url() const;
    QString submitUrl() const;
    QString key() const;
    QString id() const;
    QString verifier() const;
    QString nonce() const;
    QString realm() const;
    bool CheckVerifier(const QString& key) const;

private:
    void setRequestType(const QString& requestType);
    void setSortSelection(bool sortSelection);
    void setLogin(const QString& login);
    void setPassword(const QString& password);
    void setUuid(const QString& uuid);
    void setUrl(const QString& url);
    void setSubmitUrl(const QString& submitUrl);
    void setKey(const QString& key);
    void setId(const QString& id);
    void setVerifier(const QString& verifier);
    void setNonce(const QString& nonce);
    void setRealm(const QString& realm);

    QString m_requestType;
    bool m_sortSelection;
    QString m_login;
    QString m_password;
    QString m_uuid;
    QString m_url;
    QString m_submitUrl;
    QString m_key;
    QString m_id;
    QString m_verifier;
    QString m_nonce;
    QString m_realm;
    mutable SymmetricCipherGcrypt m_cipher;
};

} // namespace KeepassHttpProtocol

#endif // REQUEST_H
