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

PasswordGenerator::PasswordGenerator()
    : m_length(0)
    , m_classes(0)
    , m_flags(0)
{
}

int PasswordGenerator::calculateEntropy(QString password)
{
    QString delimiters = " _.,-:|";
    QString pass;
    QChar best_delimiter;
    int repetitions;

    // goes through each character in the 'delimiters' string and uses it
    // as a token to seperate the password into smaller tokens, so as to
    // detect repetitious portions of the password.
    QList<int> sizes;
    for (int i = 0; i < delimiters.size(); i++) {
        QStringList segments(password.split(delimiters.at(i), QString::SkipEmptyParts));
        if (segments.size() > 1) {
            sizes.append(segments.count(segments.at(i)));
            continue;
        }

        // no repetitions using delimiters[i].
        sizes.append(0);
    }

    // cycles through the list of sizes to determine which
    // delimiter's set of strings yielded the most repetitions.
    repetitions = 0;
    for (int i = 0; i < delimiters.size(); ++i) {
        if (sizes.at(i) > repetitions) {
            repetitions = sizes.at(i);
            best_delimiter = delimiters.at(i);
        }
    }

    // if no repetitions were found in the password (password_password with an
    // underscore as a delimiter for example), assign password passed to the
    // method to the 'pass' variable, which is the one used to calculate raw
    // entropy from.
    if (repetitions > 0) {
        QStringList segments(password.split(best_delimiter, QString::SkipEmptyParts));
        pass = segments.at(0);
    } else {
        pass = password;
    }

    // derive the raw entropy of smaller segment of the password
    float adjusted_entropy = calculateRawEntropy(pass);

    // at this point you would compare against a common password list

    // give a slight bonus for repeating a pattern
    for (int i = 0; i < repetitions; ++i) {
        adjusted_entropy = adjusted_entropy * 1.3;
    }

    return static_cast<int>(adjusted_entropy);
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

PasswordGenerator::CharClass PasswordGenerator::characterType(QChar c)
{
    if (c.isLower())
        return LowerLetters;
    if (c.isUpper())
        return UpperLetters;
    if (c.isNumber())
        return Numbers;

    return SpecialCharacters;
}

QVector<PasswordGroup> PasswordGenerator::passwordGroups() const
{
    QVector<PasswordGroup> passwordGroups;

    if (m_classes & LowerLetters) {
        PasswordGroup group;

        for (int i = 97; i < (97 + 26); i++) {
            if ((m_flags & ExcludeLookAlike) && (i == 108)) { // "l"
                continue;
            }

            group.append(i);
        }

        passwordGroups.append(group);
    }
    if (m_classes & UpperLetters) {
        PasswordGroup group;

        for (int i = 65; i < (65 + 26); i++) {
            if ((m_flags & ExcludeLookAlike) && (i == 73 || i == 79)) { // "I" and "O"
                continue;
            }

            group.append(i);
        }

        passwordGroups.append(group);
    }
    if (m_classes & Numbers) {
        PasswordGroup group;

        for (int i = 48; i < (48 + 10); i++) {
            if ((m_flags & ExcludeLookAlike) && (i == 48 || i == 49)) { // "0" and "1"
                continue;
            }

            group.append(i);
        }

        passwordGroups.append(group);
    }
    if (m_classes & SpecialCharacters) {
        PasswordGroup group;

        for (int i = 33; i <= 47; i++) {
            group.append(i);
        }

        for (int i = 58; i <= 64; i++) {
            group.append(i);
        }

        for (int i = 91; i <= 96; i++) {
            group.append(i);
        }

        for (int i = 123; i <= 126; i++) {
            if ((m_flags & ExcludeLookAlike) && (i == 124)) { // "|"
                continue;
            }

            group.append(i);
        }

        passwordGroups.append(group);
    }

    return passwordGroups;
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

int PasswordGenerator::calculateRawEntropy(QString password)
{
    PasswordGenerator::CharClass type, last_type;
    QChar c, last_c;

    // doesn't exist; sentinal value.
    last_type = static_cast<PasswordGenerator::CharClass>(0x0);
    last_c = '\0';

    int type_count = 0;
    int char_count = 0;

    float entropy = 0.0f;
    float val = 0.0f;

    for (int i = 0; i < password.size(); i++) {
        c = password.at(i);
        type = characterType(c);

        if (c == last_c) {
            char_count++;
        } else {
            char_count = 1;
        }

        if (type == last_type) {
            type_count++;
        } else {
            type_count = 0;
        }

        // these can be tweaked if someone more knowledgeable comes along
        switch (type_count) {
        case 0:
            val = 7.0f / char_count;
            break;
        case 1:
            val = 5.0f / char_count;
            break;
        case 2:
            val = 3.5f / char_count;
            break;
        case 3:
            val = 2.75f / char_count;
            break;
        case 4:
            val = 2.0f / char_count;
            break;
        case 5:
            val = 1.5f / char_count;
            break;
        case 6:
            val = 0.75f / char_count;
            break;
        default:
            val = 0.5f / char_count;
        }

        entropy += val;
        last_type = type;
    }

    entropy += password.size() / 2;

    return static_cast<int>(entropy);
}
