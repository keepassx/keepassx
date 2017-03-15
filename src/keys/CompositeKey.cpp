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

#include "CompositeKey.h"
#include "CompositeKey_p.h"

#include <QtConcurrent>
#include <QElapsedTimer>
#include <crypto/kdf/Kdf.h>
#include <crypto/kdf/Argon2Kdf.h>
#include <format/KeePass2.h>

#include "core/Global.h"
#include "crypto/CryptoHash.h"
#include "crypto/SymmetricCipher.h"

CompositeKey::CompositeKey()
{
}

CompositeKey::CompositeKey(const CompositeKey& key)
{
    *this = key;
}

CompositeKey::~CompositeKey()
{
    clear();
}

void CompositeKey::clear()
{
    qDeleteAll(m_keys);
    m_keys.clear();
}

bool CompositeKey::isEmpty() const
{
    return m_keys.isEmpty();
}

CompositeKey* CompositeKey::clone() const
{
    return new CompositeKey(*this);
}

CompositeKey& CompositeKey::operator=(const CompositeKey& key)
{
    // handle self assignment as that would break when calling clear()
    if (this == &key) {
        return *this;
    }

    clear();

    for (const Key* subKey : asConst(key.m_keys)) {
        addKey(*subKey);
    }

    return *this;
}

QByteArray CompositeKey::rawKey() const
{
    CryptoHash cryptoHash(CryptoHash::Sha256);

    for (const Key* key : m_keys) {
        cryptoHash.addData(key->rawKey());
    }

    return cryptoHash.result();
}

QByteArray CompositeKey::transform(QVariantMap kdfParams, bool* ok, QString* errorString) const
{
    QScopedPointer<Kdf> kdf(Kdf::getKdf(kdfParams));
    if (kdf.isNull()) {
        *ok = false;
        *errorString = "Unsupported KDF";
        return QByteArray();
    }

    return kdf->transform(rawKey(), kdfParams, ok, errorString);
}

void CompositeKey::addKey(const Key& key)
{
    m_keys.append(key.clone());
}

int CompositeKey::transformKeyAesBenchmark(int msec)
{
    TransformKeyAesBenchmarkThread thread1(msec);
    TransformKeyAesBenchmarkThread thread2(msec);

    thread1.start();
    thread2.start();

    thread1.wait();
    thread2.wait();

    return qMin(thread1.rounds(), thread2.rounds());
}

int CompositeKey::transformKeyArgon2Benchmark(int msec, int lanes, int memory)
{
    TransformKeyArgon2BenchmarkThread thread1(msec, lanes, memory);
    TransformKeyArgon2BenchmarkThread thread2(msec, lanes, memory);

    thread1.start();
    thread2.start();

    thread1.wait();
    thread2.wait();

    return qMin(thread1.rounds(), thread2.rounds());
}

TransformKeyAesBenchmarkThread::TransformKeyAesBenchmarkThread(int msec)
    : m_msec(msec)
    , m_rounds(0)
{
    Q_ASSERT(msec > 0);
}

int TransformKeyAesBenchmarkThread::rounds()
{
    return m_rounds;
}

void TransformKeyAesBenchmarkThread::run()
{
    QByteArray key = QByteArray(16, '\x7E');
    QByteArray seed = QByteArray(32, '\x4B');
    QByteArray iv(16, 0);

    SymmetricCipher cipher(SymmetricCipher::Aes256, SymmetricCipher::Ecb,
                           SymmetricCipher::Encrypt);
    cipher.init(seed, iv);

    QElapsedTimer t;
    t.start();

    do {
        if (!cipher.processInPlace(key, 10000)) {
            m_rounds = -1;
            return;
        }
        m_rounds += 10000;
    } while (!t.hasExpired(m_msec));
}

TransformKeyArgon2BenchmarkThread::TransformKeyArgon2BenchmarkThread(int msec, int lanes, int memory)
        : m_msec(msec)
        , m_lanes(lanes)
        , m_memory(memory)
        , m_rounds(0)
{
    Q_ASSERT(msec > 0);
    Q_ASSERT(lanes > 0);
    Q_ASSERT(memory >= 8192);
}

int TransformKeyArgon2BenchmarkThread::rounds() {
    return m_rounds;
}

void TransformKeyArgon2BenchmarkThread::run() {
    QByteArray key = QByteArray(32, '\x7E');
    Argon2Kdf kdf;
    QVariantMap p = kdf.defaultParams();
    kdf.randomizeSalt(p);
    p.insert(KeePass2::KDFPARAM_ARGON2_LANES, m_lanes);
    p.insert(KeePass2::KDFPARAM_ARGON2_MEMORY, m_memory);

    QElapsedTimer t;
    bool ok;
    QString err;
    t.start();

    do {
        kdf.transform(key, p, &ok, &err);
        m_rounds++;
    } while (!t.hasExpired(m_msec));
}
