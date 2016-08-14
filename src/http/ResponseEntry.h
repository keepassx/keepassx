/*
 *  Copyright (C) 2013 Francois Ferrand <thetypz@gmail.com>
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

#ifndef RESPONSE_ENTRY_H
#define RESPONSE_ENTRY_H

#include <QtCore>

namespace KeepassHttpProtocol {

class ResponseEntry : public QObject
{
    Q_OBJECT
    Q_PROPERTY(QString Login    READ login   )
    Q_PROPERTY(QString Password READ password)
    Q_PROPERTY(QString Uuid     READ uuid    )
    Q_PROPERTY(QString Name     READ name    )

public:
    ResponseEntry();
    ResponseEntry(QString name, QString login, QString password, QString uuid);
    ResponseEntry(const ResponseEntry& other);
    ResponseEntry& operator=(const ResponseEntry& other);

    QString login() const;
    QString password() const;
    QString uuid() const;
    QString name() const;

private:
    QString m_login;
    QString m_password;
    QString m_uuid;
    QString m_name;
};

} // namespace KeepassHttpProtocol

#endif // RESPONSE_ENTRY_H
