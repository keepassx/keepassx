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

#include "Crypto.h"

#include <QMutex>

#include <gcrypt.h>

#include "config-keepassx.h"
#include "crypto/CryptoHash.h"
#include "crypto/SymmetricCipher.h"

bool Crypto::m_initalized(false);
QString Crypto::m_errorStr;

#if !defined(GCRYPT_VERSION_NUMBER) || (GCRYPT_VERSION_NUMBER < 0x010600)
static int gcry_qt_mutex_init(void** p_sys)
{
    *p_sys = new QMutex();
    return 0;
}

static int gcry_qt_mutex_destroy(void** p_sys)
{
    delete reinterpret_cast<QMutex*>(*p_sys);
    return 0;
}

static int gcry_qt_mutex_lock(void** p_sys)
{
    reinterpret_cast<QMutex*>(*p_sys)->lock();
    return 0;
}

static int gcry_qt_mutex_unlock(void** p_sys)
{
    reinterpret_cast<QMutex*>(*p_sys)->unlock();
    return 0;
}

static const struct gcry_thread_cbs gcry_threads_qt =
{
    GCRY_THREAD_OPTION_USER,
    0,
    gcry_qt_mutex_init,
    gcry_qt_mutex_destroy,
    gcry_qt_mutex_lock,
    gcry_qt_mutex_unlock,
    0, 0, 0, 0, 0, 0, 0, 0
};
#endif

Crypto::Crypto()
{
}

bool Crypto::init()
{
    if (m_initalized) {
        qWarning("Crypto::init: already initalized");
        return true;
    }

    // libgcrypt >= 1.6 doesn't allow custom thread callbacks anymore.
#if !defined(GCRYPT_VERSION_NUMBER) || (GCRYPT_VERSION_NUMBER < 0x010600)
    gcry_control(GCRYCTL_SET_THREAD_CBS, &gcry_threads_qt);
#endif
    gcry_check_version(0);
    gcry_control(GCRYCTL_INITIALIZATION_FINISHED, 0);

    if (!checkAlgorithms()) {
        return false;
    }

    // has to be set before testing Crypto classes
    m_initalized = true;

    if (!selfTest()) {
        m_initalized = false;
        return false;
    }

    return true;
}

bool Crypto::initalized()
{
    return m_initalized;
}

QString Crypto::errorString()
{
    return m_errorStr;
}

bool Crypto::backendSelfTest()
{
    return (gcry_control(GCRYCTL_SELFTEST) == 0);
}

bool Crypto::checkAlgorithms()
{
    if (gcry_cipher_algo_info(GCRY_CIPHER_AES256, GCRYCTL_TEST_ALGO, Q_NULLPTR, Q_NULLPTR) != 0) {
        m_errorStr = "GCRY_CIPHER_AES256 not found.";
        qWarning("Crypto::checkAlgorithms: %s", qPrintable(m_errorStr));
        return false;
    }
    if (gcry_cipher_algo_info(GCRY_CIPHER_TWOFISH, GCRYCTL_TEST_ALGO, Q_NULLPTR, Q_NULLPTR) != 0) {
        m_errorStr = "GCRY_CIPHER_TWOFISH not found.";
        qWarning("Crypto::checkAlgorithms: %s", qPrintable(m_errorStr));
        return false;
    }
#ifdef GCRYPT_HAS_SALSA20
    if (gcry_cipher_algo_info(GCRY_CIPHER_SALSA20, GCRYCTL_TEST_ALGO, Q_NULLPTR, Q_NULLPTR) != 0) {
        m_errorStr = "GCRY_CIPHER_SALSA20 not found.";
        qWarning("Crypto::checkAlgorithms: %s", qPrintable(m_errorStr));
        return false;
    }
#endif
    if (gcry_md_test_algo(GCRY_MD_SHA256) != 0) {
        m_errorStr = "GCRY_MD_SHA256 not found.";
        qWarning("Crypto::checkAlgorithms: %s", qPrintable(m_errorStr));
        return false;
    }

    return true;
}

bool Crypto::selfTest()
{
    return testSha256() && testAes256Cbc() && testAes256Ecb() && testTwofish() && testSalsa20();
}

void Crypto::raiseError(const QString& str)
{
    m_errorStr = str;
    qWarning("Crypto::selfTest: %s", qPrintable(m_errorStr));
}

bool Crypto::testSha256()
{
    QByteArray sha256Test = CryptoHash::hash("abcdbcdecdefdefgefghfghighijhijkijkljklmklmnlmnomnopnopq",
                                             CryptoHash::Sha256);

    if (sha256Test != QByteArray::fromHex("248D6A61D20638B8E5C026930C3E6039A33CE45964FF2167F6ECEDD419DB06C1")) {
        raiseError("SHA-256 mismatch.");
        return false;
    }

    return true;
}

bool Crypto::testAes256Cbc()
{
    QByteArray key = QByteArray::fromHex("603deb1015ca71be2b73aef0857d77811f352c073b6108d72d9810a30914dff4");
    QByteArray iv = QByteArray::fromHex("000102030405060708090a0b0c0d0e0f");
    QByteArray plainText = QByteArray::fromHex("6bc1bee22e409f96e93d7e117393172a");
    plainText.append(QByteArray::fromHex("ae2d8a571e03ac9c9eb76fac45af8e51"));
    QByteArray cipherText = QByteArray::fromHex("f58c4c04d6e5f1ba779eabfb5f7bfbd6");
    cipherText.append(QByteArray::fromHex("9cfc4e967edb808d679f777bc6702c7d"));
    bool ok;

    SymmetricCipher aes256Encrypt(SymmetricCipher::Aes256, SymmetricCipher::Cbc, SymmetricCipher::Encrypt);
    if (!aes256Encrypt.init(key, iv)) {
        raiseError(aes256Encrypt.errorString());
        return false;
    }
    QByteArray encryptedText = aes256Encrypt.process(plainText, &ok);
    if (!ok) {
        raiseError(aes256Encrypt.errorString());
        return false;
    }
    if (encryptedText != cipherText) {
        raiseError("AES-256 CBC encryption mismatch.");
        return false;
    }

    SymmetricCipher aes256Descrypt(SymmetricCipher::Aes256, SymmetricCipher::Cbc, SymmetricCipher::Decrypt);
    if (!aes256Descrypt.init(key, iv)) {
        raiseError(aes256Descrypt.errorString());
        return false;
    }
    QByteArray decryptedText = aes256Descrypt.process(cipherText, &ok);
    if (!ok) {
        raiseError(aes256Descrypt.errorString());
        return false;
    }
    if (decryptedText != plainText) {
        raiseError("AES-256 CBC decryption mismatch.");
        return false;
    }

    return true;
}

bool Crypto::testAes256Ecb()
{
    QByteArray key = QByteArray::fromHex("000102030405060708090A0B0C0D0E0F101112131415161718191A1B1C1D1E1F");
    QByteArray iv = QByteArray::fromHex("00000000000000000000000000000000");
    QByteArray plainText = QByteArray::fromHex("00112233445566778899AABBCCDDEEFF");
    plainText.append(QByteArray::fromHex("00112233445566778899AABBCCDDEEFF"));
    QByteArray cipherText = QByteArray::fromHex("8EA2B7CA516745BFEAFC49904B496089");
    cipherText.append(QByteArray::fromHex("8EA2B7CA516745BFEAFC49904B496089"));
    bool ok;

    SymmetricCipher aes256Encrypt(SymmetricCipher::Aes256, SymmetricCipher::Ecb, SymmetricCipher::Encrypt);
    if (!aes256Encrypt.init(key, iv)) {
        raiseError(aes256Encrypt.errorString());
        return false;
    }
    QByteArray encryptedText = aes256Encrypt.process(plainText, &ok);
    if (!ok) {
        raiseError(aes256Encrypt.errorString());
        return false;
    }
    if (encryptedText != cipherText) {
        raiseError("AES-256 ECB encryption mismatch.");
        return false;
    }

    SymmetricCipher aes256Descrypt(SymmetricCipher::Aes256, SymmetricCipher::Ecb, SymmetricCipher::Decrypt);
    if (!aes256Descrypt.init(key, iv)) {
        raiseError(aes256Descrypt.errorString());
        return false;
    }
    QByteArray decryptedText = aes256Descrypt.process(cipherText, &ok);
    if (!ok) {
        raiseError(aes256Descrypt.errorString());
        return false;
    }
    if (decryptedText != plainText) {
        raiseError("AES-256 ECB decryption mismatch.");
        return false;
    }

    return true;
}

bool Crypto::testTwofish()
{
    QByteArray key = QByteArray::fromHex("603deb1015ca71be2b73aef0857d77811f352c073b6108d72d9810a30914dff4");
    QByteArray iv = QByteArray::fromHex("000102030405060708090a0b0c0d0e0f");
    QByteArray plainText = QByteArray::fromHex("6bc1bee22e409f96e93d7e117393172a");
    plainText.append(QByteArray::fromHex("ae2d8a571e03ac9c9eb76fac45af8e51"));
    QByteArray cipherText = QByteArray::fromHex("e0227c3cc80f3cb1b2ed847cc6f57d3c");
    cipherText.append(QByteArray::fromHex("657b1e7960b30fb7c8d62e72ae37c3a0"));
    bool ok;

    SymmetricCipher twofishEncrypt(SymmetricCipher::Twofish, SymmetricCipher::Cbc, SymmetricCipher::Encrypt);
    if (!twofishEncrypt.init(key, iv)) {
        raiseError(twofishEncrypt.errorString());
        return false;
    }
    QByteArray encryptedText = twofishEncrypt.process(plainText, &ok);
    if (!ok) {
        raiseError(twofishEncrypt.errorString());
        return false;
    }
    if (encryptedText != cipherText) {
        raiseError("Twofish encryption mismatch.");
        return false;
    }


    SymmetricCipher twofishDecrypt(SymmetricCipher::Twofish, SymmetricCipher::Cbc, SymmetricCipher::Decrypt);
    if (!twofishDecrypt.init(key, iv)) {
        raiseError(twofishEncrypt.errorString());
        return false;
    }
    QByteArray decryptedText = twofishDecrypt.process(cipherText, &ok);
    if (!ok) {
        raiseError(twofishDecrypt.errorString());
        return false;
    }
    if (decryptedText != plainText) {
        raiseError("Twofish encryption mismatch.");
        return false;
    }

    return true;
}

bool Crypto::testSalsa20()
{
    QByteArray salsa20Key = QByteArray::fromHex("F3F4F5F6F7F8F9FAFBFCFDFEFF000102030405060708090A0B0C0D0E0F101112");
    QByteArray salsa20iv = QByteArray::fromHex("0000000000000000");
    QByteArray salsa20Plain = QByteArray::fromHex("00000000000000000000000000000000");
    QByteArray salsa20Cipher = QByteArray::fromHex("B4C0AFA503BE7FC29A62058166D56F8F");
    bool ok;

    SymmetricCipher salsa20Stream(SymmetricCipher::Salsa20, SymmetricCipher::Stream,
                                  SymmetricCipher::Encrypt);
    if (!salsa20Stream.init(salsa20Key, salsa20iv)) {
        raiseError(salsa20Stream.errorString());
        return false;
    }

    QByteArray salsaProcessed = salsa20Stream.process(salsa20Plain, &ok);
    if (!ok) {
        raiseError(salsa20Stream.errorString());
        return false;
    }
    if (salsaProcessed != salsa20Cipher) {
        raiseError("Salsa20 stream cipher mismatch.");
        return false;
    }

    return true;
}
