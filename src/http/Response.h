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

#ifndef RESPONSE_H
#define RESPONSE_H

#include "Protocol.h"
#include "Request.h"
#include "ResponseEntry.h"
#include <QtCore>
#include "crypto/SymmetricCipherGcrypt.h"

namespace KeepassHttpProtocol {

class Response : public QObject
{
    Q_OBJECT
    Q_PROPERTY(QString RequestType READ requestTypeStr)
    Q_PROPERTY(QString Error       READ error         )
    Q_PROPERTY(bool    Success     READ success       )
    Q_PROPERTY(QString Id          READ id            )
    Q_PROPERTY(QString Version     READ version       )
    Q_PROPERTY(QString Hash        READ hash          )
    Q_PROPERTY(QVariant Count      READ count         )
    Q_PROPERTY(QVariant Entries    READ getEntries    )
    Q_PROPERTY(QString Nonce       READ nonce         )
    Q_PROPERTY(QString Verifier    READ verifier      )

public:
    Response(const Request& request, QString hash);

    RequestType requestType() const;
    QString error() const;
    void setError(const QString& error = QString());
    bool success() const;
    void setSuccess();
    QString id() const;
    void setId(const QString& id);
    QString version() const;
    QString hash() const;
    QVariant count() const;
    void setCount(int count);
    QVariant getEntries() const;
    void setEntries(const QList<ResponseEntry>& entries);
    QString nonce() const;
    QString verifier() const;
    void setVerifier(QString key);

    QString toJson();

private:
    QString requestTypeStr() const;

    QString m_requestType;
    QString m_error;
    bool    m_success;
    QString m_id;
    int     m_count;
    QString m_version;
    QString m_hash;
    QList<ResponseEntry> m_entries;
    QString m_nonce;
    QString m_verifier;
    SymmetricCipherGcrypt m_cipher;
};

} // namespace KeepassHttpProtocol

#endif // RESPONSE_H
