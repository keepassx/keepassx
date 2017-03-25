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

#ifndef SERVICE_H
#define SERVICE_H

#include "Server.h"
#include <QObject>
#include "gui/DatabaseTabWidget.h"

class Service : public KeepassHttpProtocol::Server {
    Q_OBJECT

public:
    static constexpr const char* defaultHost = "127.0.0.1";
    static constexpr const int defaultPort = 19455;

    explicit Service(DatabaseTabWidget *parent = 0);
    Service(DatabaseTabWidget *parent, const char* host, int port);

    virtual bool isDatabaseOpened() const;
    virtual bool openDatabase();
    virtual QString getDatabaseRootUuid();
    virtual QString getDatabaseRecycleBinUuid();
    virtual QString getKey(const QString& id);
    virtual QString storeKey(const QString& key);
    virtual QList<KeepassHttpProtocol::ResponseEntry> findMatchingEntries(const QString& id, const QString& url, const QString& submitUrl, const QString& realm);
    virtual int countMatchingEntries(const QString& id, const QString& url, const QString& submitUrl, const QString& realm);
    virtual QList<KeepassHttpProtocol::ResponseEntry> searchAllEntries(const QString& id);
    virtual void addEntry(const QString& id, const QString& login, const QString& password, const QString& url, const QString& submitUrl, const QString& realm);
    virtual void updateEntry(const QString& id, const QString& uuid, const QString& login, const QString& password, const QString& url);

private:
    Entry* getConfigEntry(bool create = false);
    bool matchUrlScheme(const QString& url);
    bool removeFirstDomain(QString& hostname);
    Group* findCreateAddEntryGroup();
    QList<Entry*> searchEntries(const QString& text);

    DatabaseTabWidget* const m_dbTabWidget;
};

#endif // SERVICE_H
