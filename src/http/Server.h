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

#ifndef SERVER_H
#define SERVER_H

#include <memory>
#include <QObject>
#include <QList>
#include <evhttp.h>

namespace KeepassHttpProtocol {

class Request;
class Response;
class ResponseEntry;
class HttpServer;

class Server : public QObject
{
    Q_OBJECT
public:
    explicit Server(QObject* parent = 0);
    virtual ~Server();

    // TODO: use QByteArray?
    virtual bool isDatabaseOpened() const = 0;
    virtual bool openDatabase() = 0;
    virtual QString getDatabaseRootUuid() = 0;
    virtual QString getDatabaseRecycleBinUuid() = 0;
    virtual QString getKey(const QString& id) = 0;
    virtual QString storeKey(const QString& key) = 0;
    virtual QList<ResponseEntry> findMatchingEntries(const QString& id, const QString& url, const QString&  submitUrl, const QString&  realm) = 0;
    virtual int countMatchingEntries(const QString& id, const QString& url, const QString&  submitUrl, const QString&  realm) = 0;
    virtual QList<ResponseEntry> searchAllEntries(const QString& id) = 0;
    virtual void addEntry(const QString& id, const QString& login, const QString& password, const QString& url, const QString& submitUrl, const QString& realm) = 0;
    virtual void updateEntry(const QString& id, const QString& uuid, const QString& login, const QString& password, const QString& url) = 0;

public Q_SLOTS:
    void start(const char* host, int port);
    void stop();
    bool isStarted() const;

private Q_SLOTS:
    void acceptRequest(evhttp_request* request, bool* accept);
    void handleRequest(const QByteArray request, QByteArray* response, int* status);

private:
    void testAssociate(const Request& r, Response* protocolResp);
    void associate(const Request& r, Response* protocolResp);
    void getLogins(const Request& r, Response* protocolResp);
    void getLoginsCount(const Request& r, Response* protocolResp);
    void getAllLogins(const Request& r, Response* protocolResp);
    void setLogin(const Request& r, Response* protocolResp);
    void generatePassword(const Request& r, Response* protocolResp);

    std::unique_ptr<HttpServer> m_httpServer;
    bool m_started;
};

}  // namespace KeepassHttpProtocol

#endif // SERVER_H
