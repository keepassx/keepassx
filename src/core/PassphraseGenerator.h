/*
 *  Copyright (C) 2013 Felix Geyer <debfx@fobos.de>
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

#ifndef KEEPASSX_PASSPHRASEGENERATOR_H
#define KEEPASSX_PASSPHRASEGENERATOR_H

#include <QFlags>
#include <QString>
#include <QVector>

class PassphraseGenerator
{
public:
    PassphraseGenerator();

    void setLength(int length);
    bool isValid() const;

    QString generatePassphrase() const;

private:
    int m_length;
    QVector<QString> m_wordlist;

    Q_DISABLE_COPY(PassphraseGenerator)
};

#endif // KEEPASSX_PASSPHRASEGENERATOR_H
