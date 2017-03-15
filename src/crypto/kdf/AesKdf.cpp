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

#include <format/KeePass2.h>
#include <QtConcurrent>
#include <crypto/SymmetricCipher.h>
#include <crypto/CryptoHash.h>
#include <crypto/Random.h>
#include "AesKdf.h"

void AesKdf::randomizeSalt(QVariantMap &p) {
    p.insert(KeePass2::KDFPARAM_AES_SEED, randomGen()->randomArray(32));
}

QByteArray AesKdf::transform(QByteArray key, QVariantMap p, bool* ok, QString* errorString) {
    quint64 rounds = p.value(KeePass2::KDFPARAM_AES_ROUNDS).toULongLong(ok);
    if (!*ok) {
        *errorString = "Invalid AES rounds parameter";
        return QByteArray();
    }
    Q_ASSERT(rounds > 0);

    QByteArray seed = p.value(KeePass2::KDFPARAM_AES_SEED).toByteArray();
    Q_ASSERT(seed.size() == 32);
    if (seed.size() != 32) {
        *ok = false;
        *errorString = "Invalid AES seed";
        return QByteArray();
    }

    bool okLeft;
    QString errorStringLeft;
    bool okRight;
    QString errorStringRight;

    QFuture<QByteArray> future = QtConcurrent::run(transformKeyRaw, key.left(16), seed, rounds,
                                                   &okLeft, &errorStringLeft);
    QByteArray result2 = transformKeyRaw(key.right(16), seed, rounds, &okRight, &errorStringRight);

    QByteArray transformed;
    transformed.append(future.result());
    transformed.append(result2);

    *ok = (okLeft && okRight);

    if (!okLeft) {
        *errorString = errorStringLeft;
        return QByteArray();
    }

    if (!okRight) {
        *errorString = errorStringRight;
        return QByteArray();
    }

    return CryptoHash::hash(transformed, CryptoHash::Sha256);
}

QByteArray AesKdf::transformKeyRaw(const QByteArray& key, const QByteArray& seed,
                                         quint64 rounds, bool* ok, QString* errorString)
{
    QByteArray iv(16, 0);
    SymmetricCipher cipher(SymmetricCipher::Aes256, SymmetricCipher::Ecb,
                           SymmetricCipher::Encrypt);
    if (!cipher.init(seed, iv)) {
        *ok = false;
        *errorString = cipher.errorString();
        return QByteArray();
    }

    QByteArray result = key;

    if (!cipher.processInPlace(result, rounds)) {
        *ok = false;
        *errorString = cipher.errorString();
        return QByteArray();
    }

    *ok = true;
    return result;
}

QVariantMap AesKdf::defaultParams() {
    QVariantMap params;
    params.insert(KeePass2::KDFPARAM_UUID, QVariant(KeePass2::KDF_AES.toByteArray()));
    params.insert(KeePass2::KDFPARAM_AES_ROUNDS, QVariant(100000ull));
    return params;
}
