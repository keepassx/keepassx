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

#ifndef KEEPASSX_KEEPASS2_H
#define KEEPASSX_KEEPASS2_H

#include <QtGlobal>
#include <QMap>
#include <crypto/SymmetricCipher.h>

#include "core/Uuid.h"

namespace KeePass2
{
    const quint32 SIGNATURE_1 = 0x9AA2D903;
    const quint32 SIGNATURE_2 = 0xB54BFB67;

    const quint32 FILE_VERSION_MIN = 0x00020000;
    const quint32 FILE_VERSION_CRITICAL_MASK = 0xFFFF0000;
    const quint32 FILE_VERSION_4 = 0x00040000;
    const quint32 FILE_VERSION_3 = 0x00030001;
    const quint32 FILE_VERSION = FILE_VERSION_4;

    const quint16 VARIANTMAP_VERSION = 0x0100;
    const quint16 VARIANTMAP_CRITICAL_MASK = 0xFF00;

    const QSysInfo::Endian BYTEORDER = QSysInfo::LittleEndian;

    const Uuid CIPHER_AES = Uuid(QByteArray::fromHex("31c1f2e6bf714350be5805216afc5aff"));
    const Uuid CIPHER_CHACHA20 = Uuid(QByteArray::fromHex("D6038A2B8B6F4CB5A524339A31DBB59A"));

    const QByteArray INNER_STREAM_SALSA20_IV("\xE8\x30\x09\x4B\x97\x20\x5D\x2A");

    const QString KDFPARAM_UUID("$UUID");
    const QString KDFPARAM_AES_ROUNDS("R");
    const QString KDFPARAM_AES_SEED("S");
    const QString KDFPARAM_ARGON2_SALT("S");
    const QString KDFPARAM_ARGON2_LANES("P");
    const QString KDFPARAM_ARGON2_MEMORY("M");
    const QString KDFPARAM_ARGON2_TIME("I");
    const QString KDFPARAM_ARGON2_VERSION("V");
    const QString KDFPARAM_ARGON2_SECRET("K");
    const QString KDFPARAM_ARGON2_ASSOCDATA("A");

    const Uuid KDF_AES = Uuid(QByteArray::fromHex("C9D9F39A628A4460BF740D08C18A4FEA"));
    const Uuid KDF_ARGON2 = Uuid(QByteArray::fromHex("EF636DDF8C29444B91F7A9A403E30A0C"));

    enum HeaderFieldID
    {
        EndOfHeader = 0,
        Comment = 1,
        CipherID = 2,
        CompressionFlags = 3,
        MasterSeed = 4,
        TransformSeed = 5,
        TransformRounds = 6,
        EncryptionIV = 7,
        ProtectedStreamKey = 8,
        StreamStartBytes = 9,
        InnerRandomStreamID = 10,
        KdfParameters = 11,
        PublicCustomData = 12
    };

    enum class InnerHeaderFieldID : quint8
    {
        End = 0,
        InnerRandomStreamID = 1,
        InnerRandomStreamKey = 2,
        Binary = 3
    };

    enum ProtectedStreamAlgo
    {
        ArcFourVariant = 1,
        Salsa20 = 2,
        ChaCha20 = 3
    };

    enum class VariantMapFieldType : quint8
    {
        End = 0,
        // Byte = 0x02,
        // UInt16 = 0x03,
        UInt32 = 0x04,
        UInt64 = 0x05,
        // Signed mask: 0x08
        Bool = 0x08,
        // SByte = 0x0A,
        // Int16 = 0x0B,
        Int32 = 0x0C,
        Int64 = 0x0D,
        // Float = 0x10,
        // Double = 0x11,
        // Decimal = 0x12,
        // Char = 0x17, // 16-bit Unicode character
        String = 0x18,
        // Array mask: 0x40
        ByteArray = 0x42
    };

    QByteArray hmacKey(QByteArray masterSeed, QByteArray transformedMasterKey);
    SymmetricCipher::Algorithm cipherUuidToAlgo(Uuid cipher);
    SymmetricCipher::Mode getAlgoMode(SymmetricCipher::Algorithm algo);
    int getAlgoIvSize(SymmetricCipher::Algorithm algo);
}

#endif // KEEPASSX_KEEPASS2_H
