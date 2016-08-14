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

#ifndef PROTOCOL_H
#define PROTOCOL_H

#include <QtCore>
#include "crypto/SymmetricCipherGcrypt.h"

namespace KeepassHttpProtocol {

enum RequestType {
    INVALID = -1,
    GET_LOGINS,
    GET_LOGINS_COUNT,
    GET_ALL_LOGINS,
    SET_LOGIN,
    ASSOCIATE,
    TEST_ASSOCIATE,
    GENERATE_PASSWORD
};

const char* const STR_GET_LOGINS = "get-logins";
const char* const STR_GET_LOGINS_COUNT = "get-logins-count";
const char* const STR_GET_ALL_LOGINS = "get-all-logins";
const char* const STR_SET_LOGIN = "set-login";
const char* const STR_ASSOCIATE = "associate";
const char* const STR_TEST_ASSOCIATE = "test-associate";
const char* const STR_GENERATE_PASSWORD = "generate-password";
const char* const STR_VERSION = "1.8.4.1";

QHash<QString, RequestType> createStringHash();
RequestType parseRequest(const QString& str);
QByteArray decode64(QString s);
QString encode64(QByteArray b);
QByteArray decrypt2(const QByteArray& data, SymmetricCipherGcrypt& cipher);
QString decrypt(const QString& data, SymmetricCipherGcrypt& cipher);
QByteArray encrypt2(const QByteArray& data, SymmetricCipherGcrypt& cipher);
QString encrypt(const QString& data, SymmetricCipherGcrypt& cipher);

} // namespace KeepassHttpProtocol

#endif // PROTOCOL_H
