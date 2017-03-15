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

#ifndef KEEPASSX_KDF_H
#define KEEPASSX_KDF_H

#include <QVariant>
#include "core/Uuid.h"

class Kdf
{
public:
    virtual ~Kdf() {}
    virtual void randomizeSalt(QVariantMap &p) = 0;
    virtual QByteArray transform(QByteArray raw, QVariantMap p, bool* ok, QString* errorString) = 0;
    virtual QVariantMap defaultParams() = 0;

    static Kdf* getKdf(QVariantMap p);
    static Kdf* getKdf(Uuid u);
};

#endif // KEEPASSX_KDF_H
