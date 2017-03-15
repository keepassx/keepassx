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
#include "Kdf.h"
#include "AesKdf.h"
#include "Argon2Kdf.h"

Kdf* Kdf::getKdf(QVariantMap p)
{
    QByteArray uuidBytes = p.value(KeePass2::KDFPARAM_UUID).toByteArray();
    if (uuidBytes.size() != Uuid::Length) {
        return nullptr;
    }

    return getKdf(Uuid(uuidBytes));
}

Kdf* Kdf::getKdf(Uuid uuid) {
    if (uuid == KeePass2::KDF_AES) {
        return new AesKdf();
    } else if (uuid == KeePass2::KDF_ARGON2) {
        return new Argon2Kdf();
    }

    return nullptr;
}