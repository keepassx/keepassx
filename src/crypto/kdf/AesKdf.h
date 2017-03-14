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

#ifndef KEEPASSX_AESKDF_H
#define KEEPASSX_AESKDF_H

#include "Kdf.h"

class AesKdf : public Kdf
{
public:
    void randomizeSalt(QVariantMap &p) override;

    QByteArray transform(QByteArray raw, QVariantMap p, bool* ok, QString* errorString) override;

    QVariantMap defaultParams() override;

private:
    static QByteArray transformKeyRaw(const QByteArray &key, const QByteArray &seed, quint64 rounds, bool *ok, QString *errorString);
};

#endif // KEEPASSX_AESKDF_H
