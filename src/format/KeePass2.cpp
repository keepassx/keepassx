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

#include <crypto/CryptoHash.h>
#include "KeePass2.h"

QByteArray KeePass2::hmacKey(QByteArray masterSeed, QByteArray transformedMasterKey) {
    CryptoHash hmacKeyHash(CryptoHash::Sha512);
    hmacKeyHash.addData(masterSeed);
    hmacKeyHash.addData(transformedMasterKey);
    hmacKeyHash.addData(QByteArray(1, '\x01'));
    return hmacKeyHash.result();
}

SymmetricCipher::Algorithm KeePass2::cipherUuidToAlgo(Uuid cipherUuid) {
    if (cipherUuid == KeePass2::CIPHER_AES) {
        return SymmetricCipher::Aes256;
    } else if (cipherUuid == KeePass2::CIPHER_CHACHA20) {
        return SymmetricCipher::ChaCha20;
    } else {
        qWarning("Invalid cipher UUID %s", cipherUuid.toHex().toUtf8().constData());
        return static_cast<SymmetricCipher::Algorithm>(-1);
    }
}

int KeePass2::getAlgoIvSize(SymmetricCipher::Algorithm algo) {
    switch (algo) {
    case SymmetricCipher::ChaCha20:
        return 12;
    case SymmetricCipher::Aes256:
    case SymmetricCipher::Twofish:
        return 16;
    default:
        return -1;
    }
}

SymmetricCipher::Mode KeePass2::getAlgoMode(SymmetricCipher::Algorithm algo) {
    switch (algo) {
        case SymmetricCipher::ChaCha20:
            return SymmetricCipher::Stream;
        case SymmetricCipher::Aes256:
        case SymmetricCipher::Twofish:
            return SymmetricCipher::Cbc;
        default:
            return static_cast<SymmetricCipher::Mode>(-1);
    }
}
