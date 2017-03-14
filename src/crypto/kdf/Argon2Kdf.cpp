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

#include <argon2.h>
#include <format/KeePass2.h>
#include <QThread>
#include <crypto/Random.h>

#include "Argon2Kdf.h"

void Argon2Kdf::randomizeSalt(QVariantMap &p) {
    p.insert(KeePass2::KDFPARAM_ARGON2_SALT, randomGen()->randomArray(32));
}

QByteArray Argon2Kdf::transform(QByteArray raw, QVariantMap p, bool *ok, QString *errorString) {
    QByteArray salt = p.value(KeePass2::KDFPARAM_ARGON2_SALT).toByteArray();
    if (salt.size() < MIN_SALT || salt.size() > MAX_SALT) {
        *ok = false;
        *errorString = "Invalid Argon2 salt length";
        return QByteArray();
    }

    quint32 lanes = p.value(KeePass2::KDFPARAM_ARGON2_LANES).toUInt(ok);
    if (!*ok || lanes < MIN_LANES || lanes > MAX_LANES) {
        *ok = false;
        *errorString = "Invalid Argon2 number of lanes";
        return QByteArray();
    }

    quint64 memory = p.value(KeePass2::KDFPARAM_ARGON2_MEMORY).toULongLong(ok) / 1024;
    if (!*ok || memory < MIN_MEMORY || memory > MAX_MEMORY) {
        *ok = false;
        *errorString = "Invalid Argon2 memory cost";
        return QByteArray();
    }

    quint64 time = p.value(KeePass2::KDFPARAM_ARGON2_TIME).toULongLong(ok);
    if (!*ok || time < MIN_TIME || time > MAX_TIME) {
        *ok = false;
        *errorString = "Invalid Argon2 time cost";
        return QByteArray();
    }

    quint32 version = p.value(KeePass2::KDFPARAM_ARGON2_VERSION).toUInt(ok);
    if (!*ok || version < MIN_VERSION || version > MAX_VERSION) {
        *ok = false;
        *errorString = "Invalid Argon2 version";
        return QByteArray();
    }

    QByteArray secret = p.value(KeePass2::KDFPARAM_ARGON2_SECRET).toByteArray();
    QByteArray ad = p.value(KeePass2::KDFPARAM_ARGON2_ASSOCDATA).toByteArray();
    QByteArray key = QByteArray(32, '\0');

    argon2_context a2ctx = {};
    a2ctx.threads = static_cast<quint32>(QThread::idealThreadCount());
    if (a2ctx.threads > lanes) {
        a2ctx.threads = lanes; // avoid libargon2 bug
    }
    a2ctx.lanes = lanes;
    a2ctx.m_cost = static_cast<quint32>(memory);
    a2ctx.t_cost = static_cast<quint32>(time);
    a2ctx.version = version;

    a2ctx.salt = reinterpret_cast<quint8*>(salt.data());
    a2ctx.saltlen = static_cast<quint32>(salt.size());

    a2ctx.secret = reinterpret_cast<quint8*>(secret.data());
    a2ctx.secretlen = static_cast<quint32>(secret.size());

    a2ctx.ad = reinterpret_cast<quint8*>(ad.data());
    a2ctx.adlen = static_cast<quint32>(ad.size());

    a2ctx.pwd = reinterpret_cast<quint8*>(raw.data());
    a2ctx.pwdlen = static_cast<quint32>(raw.size());

    a2ctx.out = reinterpret_cast<quint8*>(key.data());
    a2ctx.outlen = static_cast<quint32>(key.size());

    a2ctx.flags = 0;

    int result = argon2d_ctx(&a2ctx);
    if (result != ARGON2_OK) {
        *ok = false;
        *errorString = "Argon2 error";
        qWarning("Argon2 error: %s", argon2_error_message(result));
        return QByteArray();
    }

    return key;
}

QVariantMap Argon2Kdf::defaultParams() {
    QVariantMap params;
    params.insert(KeePass2::KDFPARAM_UUID, QVariant(KeePass2::KDF_ARGON2.toByteArray()));
    params.insert(KeePass2::KDFPARAM_ARGON2_LANES, QVariant(DEFAULT_LANES));
    params.insert(KeePass2::KDFPARAM_ARGON2_MEMORY, QVariant(DEFAULT_MEMORY));
    params.insert(KeePass2::KDFPARAM_ARGON2_TIME, QVariant(DEFAULT_TIME));
    params.insert(KeePass2::KDFPARAM_ARGON2_VERSION, QVariant(MAX_VERSION));
    return params;
}
