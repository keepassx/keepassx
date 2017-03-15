/*
 *  Copyright (C) 2017 angelsl
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

#ifndef KEEPASSX_ARGON2KDF_H
#define KEEPASSX_ARGON2KDF_H

#include <argon2.h>

#include "Kdf.h"

class Argon2Kdf : public Kdf {
public:
    void randomizeSalt(QVariantMap &p) override;

    QByteArray transform(QByteArray raw, QVariantMap p, bool *ok, QString *errorString) override;

    QVariantMap defaultParams() override;

    static const quint32 MIN_VERSION = ARGON2_VERSION_10;
    static const quint32 MAX_VERSION = ARGON2_VERSION_13;
    static const qint32 MIN_SALT = ARGON2_MIN_SALT_LENGTH;
    static const qint32 MAX_SALT = 2147483647; // following KeePass
    static const quint64 MIN_TIME = ARGON2_MIN_TIME;
    static const quint64 MAX_TIME = ARGON2_MAX_TIME;
    static const quint64 MIN_MEMORY = ARGON2_MIN_MEMORY;
    static const quint64 MAX_MEMORY = ARGON2_MAX_MEMORY;
    static const quint32 MIN_LANES = ARGON2_MIN_LANES;
    static const quint32 MAX_LANES = ARGON2_MAX_LANES;
    static const quint64 DEFAULT_TIME = 2;
    static const quint64 DEFAULT_MEMORY = 1024 * 1024;
    static const quint32 DEFAULT_LANES = 2;
};

#endif // KEEPASSX_ARGON2KDF_H
