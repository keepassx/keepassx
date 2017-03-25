/*
 *  Copyright (C) 2016 Yong-Siang Shih <shaform@gmail.com>
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

#ifndef KEEPASSX_TESTSERVER_H
#define KEEPASSX_TESTSERVER_H

#include <QObject>
#include <QMap>
#include <QList>
#include "http/Server.h"
#include "http/ResponseEntry.h"

using namespace KeepassHttpProtocol;

class MockServer : public Server
{
    Q_OBJECT
public:
    explicit MockServer(QObject* parent = 0);
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
    void setIsDatabaseOpened(bool opened);
    void setIsDatabaseLocked(bool locked);

private:
    bool m_isDatabaseOpened;
    bool m_isDatabaseLocked;
    QMap<QString, QString> m_keys;
    QMap<QString, QList<ResponseEntry>> m_responeEntries;
};

class TestServer : public QObject
{
    Q_OBJECT

private Q_SLOTS:
    void initTestCase();
    void testTestAssociate();
    void testAssociate();
    void testGetLogins();
    void testGetLoginsCount();
    void testGetAllLogins();
    void testSetLogin();
    void testGeneratePassword();
    void cleanupTestCase();

private:
    int sendRequest(const QJsonObject& request, QJsonObject* response);
    QJsonObject getRequest(const QString& requestType, const QString& id, const QByteArray& key);

    MockServer* m_mockServer;
    const char* m_localHost;
    int m_port;
};

#endif // KEEPASSX_TESTSERVER_H
