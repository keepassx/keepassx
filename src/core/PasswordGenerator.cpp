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

#include "PasswordGenerator.h"

#include "crypto/Random.h"

#include <QHash>

PasswordGenerator::PasswordGenerator()
    : m_length(0)
    , m_classes(0)
    , m_flags(0)
{
}

void PasswordGenerator::setLength(int length)
{
    m_length = length;
}

void PasswordGenerator::setCharClasses(const CharClasses& classes)
{
    m_classes = classes;
}

void PasswordGenerator::setFlags(const GeneratorFlags& flags)
{
    m_flags = flags;
}

QString PasswordGenerator::generatePassword() const
{
    Q_ASSERT(isValid());

    QVector<PasswordGroup> groups = passwordGroups();

    QVector<QChar> passwordChars;
    Q_FOREACH (const PasswordGroup& group, groups) {
        Q_FOREACH (QChar ch, group) {
            passwordChars.append(ch);
        }
    }

    QString password;

    if (m_flags & CharFromEveryGroup) {
        for (int i = 0; i < groups.size(); i++) {
            int pos = randomGen()->randomUInt(groups[i].size());

            password.append(groups[i][pos]);
        }

        for (int i = groups.size(); i < m_length; i++) {
            int pos = randomGen()->randomUInt(passwordChars.size());

            password.append(passwordChars[pos]);
        }

        // shuffle chars
        for (int i = (password.size() - 1); i >= 1; i--) {
            int j = randomGen()->randomUInt(i + 1);

            QChar tmp = password[i];
            password[i] = password[j];
            password[j] = tmp;
        }
    }
    else {
        for (int i = 0; i < m_length; i++) {
            int pos = randomGen()->randomUInt(passwordChars.size());

            password.append(passwordChars[pos]);
        }
    }

    return password;
}

bool PasswordGenerator::isValid() const
{
    if (m_classes == 0) {
        return false;
    }
    else if (m_length == 0) {
        return false;
    }

    if ((m_flags & CharFromEveryGroup) && (m_length < numCharClasses())) {
        return false;
    }

    return true;
}

QVector<PasswordGroup> PasswordGenerator::passwordGroups() const
{
    QHash<CharClass, PasswordGroup> passwordGroups;

    typedef struct {
        char first;
        char last;
        CharClass cls;
    } ClassRange;

    static const ClassRange asciiClasses[] = {
        { 'a',  'z', LowerLetters },
        { 'A',  'Z', UpperLetters },
        { '0',  '9', Numbers },
        { '!',  '/', SpecialCharacters },
        { ':',  '@', SpecialCharacters },
        { '[',  '`', SpecialCharacters },
        { '{',  '~', SpecialCharacters },

        { 0, 0, LowerLetters }}; // Sentinel
    static const char* lookalikes = "lI1|" "O0" "6G";

    const ClassRange* range = &asciiClasses[0];

    while (range->last) {
        if (m_classes & range->cls) {
            PasswordGroup group;

            for (int i = range->first; i <= range->last; i++) {
                if ((m_flags & ExcludeLookAlike) && strchr(lookalikes, i)) {
                    continue;
                }

                group.append(i);
            }

            if (!group.isEmpty()) {
                if (!passwordGroups.contains(range->cls)) {
                    passwordGroups[range->cls] = PasswordGroup();
                }

                passwordGroups[range->cls] += group;
            }
        }

        ++range;
    }

    return passwordGroups.values().toVector();
}

int PasswordGenerator::numCharClasses() const
{
    int numClasses = 0;

    if (m_classes & LowerLetters) {
        numClasses++;
    }
    if (m_classes & UpperLetters) {
        numClasses++;
    }
    if (m_classes & Numbers) {
        numClasses++;
    }
    if (m_classes & SpecialCharacters) {
        numClasses++;
    }

    return numClasses;
}
