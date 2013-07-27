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

PasswordGenerator* PasswordGenerator::m_instance = Q_NULLPTR;

QString PasswordGenerator::generatePassword(int length,
                                            const PasswordGenerator::CharClasses& classes,
                                            const PasswordGenerator::GeneratorFlags& flags)
{
    Q_ASSERT(isValidCombination(length, classes, flags));

    QVector<PasswordGroup> groups = passwordGroups(classes, flags);

    QVector<QChar> passwordChars;
    Q_FOREACH (const PasswordGroup& group, groups) {
        Q_FOREACH (QChar ch, group) {
            passwordChars.append(ch);
        }
    }

    QString password;

    if (flags & CharFromEveryGroup) {
        for (int i = 0; i < groups.size(); i++) {
            int pos = Random::randomUInt(groups[i].size());

            password.append(groups[i][pos]);
        }

        for (int i = groups.size(); i < length; i++) {
            int pos = Random::randomUInt(passwordChars.size());

            password.append(passwordChars[pos]);
        }

        // shuffle chars
        for (int i = (password.size() - 1); i >= 1; i--) {
            int j = Random::randomUInt(i + 1);

            QChar tmp = password[i];
            password[i] = password[j];
            password[j] = tmp;
        }
    }
    else {
        for (int i = 0; i < length; i++) {
            int pos = Random::randomUInt(passwordChars.size());

            password.append(passwordChars[pos]);
        }
    }

    return password;
}

bool PasswordGenerator::isValidCombination(int length,
                                           const PasswordGenerator::CharClasses& classes,
                                           const PasswordGenerator::GeneratorFlags& flags)
{
    if (classes == 0) {
        return false;
    }
    else if (length == 0) {
        return false;
    }

    if ((flags & CharFromEveryGroup) && (length < numCharClasses(classes))) {
        return false;
    }

    return true;
}

QVector<PasswordGroup> PasswordGenerator::passwordGroups(const PasswordGenerator::CharClasses& classes,
                                                         const PasswordGenerator::GeneratorFlags& flags)
{
    QVector<PasswordGroup> passwordGroups;

    typedef struct {
        char first;
        char last;
        int cls;
    } ClassRange;

    static const ClassRange asciiClasses[] = {
        { '\0', ' ', 0 },
        { 'a',  'z', LowerLetters },
        { 'A',  'Z', UpperLetters },
        { '0',  '9', Numbers },
        { '!',  '/', SpecialCharacters },
        { ':',  '@', SpecialCharacters },
        { '[',  '`', SpecialCharacters },
        { '{',  '~', SpecialCharacters },
        { 0, 0, 0 }}; // Sentinel
    static const char* lookalikes = "lI1| O0 G6"; // Space is ignored anyways, so we can use it here to group lookalikes

    const ClassRange* range = &asciiClasses[0];

    while (range->last) {
        if (classes & range->cls) {
            PasswordGroup group;

            for (int i = range->first; i <= range->last; i++) {
                if ((flags & ExcludeLookAlike) && strchr(lookalikes, i)) {
                    continue;
                }

                group.append(i);
            }

            if (!group.isEmpty()) {
                passwordGroups.append(group);
            }
        }

        ++range;
    }

    return passwordGroups;
}

int PasswordGenerator::numCharClasses(const PasswordGenerator::CharClasses& classes)
{
    int numClasses = 0;

    if (classes & LowerLetters) {
        numClasses++;
    }
    if (classes & UpperLetters) {
        numClasses++;
    }
    if (classes & Numbers) {
        numClasses++;
    }
    if (classes & SpecialCharacters) {
        numClasses++;
    }

    return numClasses;
}

PasswordGenerator* PasswordGenerator::instance()
{
    if (!m_instance) {
        m_instance = new PasswordGenerator();
    }

    return m_instance;
}

PasswordGenerator::PasswordGenerator()
{
}
