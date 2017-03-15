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

#ifndef KEEPASSX_COMPOSITEKEY_H
#define KEEPASSX_COMPOSITEKEY_H

#include <QList>
#include <QVariant>

#include "keys/Key.h"

class CompositeKey : public Key
{
public:
    CompositeKey();
    CompositeKey(const CompositeKey& key);
    ~CompositeKey();
    void clear();
    bool isEmpty() const;
    CompositeKey* clone() const;
    CompositeKey& operator=(const CompositeKey& key);

    QByteArray rawKey() const;
    QByteArray transform(QVariantMap kdfParams, bool* ok, QString* errorString) const;
    void addKey(const Key& key);

    static int transformKeyAesBenchmark(int msec);
    static int transformKeyArgon2Benchmark(int msec, int lanes, int memory);

private:
    QList<Key*> m_keys;
};

#endif // KEEPASSX_COMPOSITEKEY_H
