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

#include "ArgumentParser.h"

QHash<QString, QString> ArgumentParser::parseArguments(const QStringList& args)
{
    QStringList argumentKeys = QStringList() << "password" << "config" << "filename";

    QHash<QString, QString> argumentMap;

    // TODO: needs refactoring, too much nesting
    for (int i = 1; i < args.size(); i++) {
        if (args[i].startsWith("--")) {
            if (args.size() > (i + 1)) {
                QString argument(args[i].mid(2));
                if (argumentKeys.contains(argument)) {
                    argumentMap.insert(argument, args[i + 1]);
                }
                else {
                    qWarning("Unknown option \"%s\" with value \"%s\"", qPrintable(args[i]), qPrintable(args[i+1]));
                }
                i++;
            } else {
                qWarning("No value given for option \"%s\"", qPrintable(args[i]));
            }
        }
        else if (!args[i].startsWith("-")) {
            argumentMap.insert("filename", args[i]);
        }
        else {
            qWarning("Unknown argument \"%s\"", qPrintable(args[i]));
        }
    }

    return argumentMap;
}
