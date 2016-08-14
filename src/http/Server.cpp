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

#include <evhttp.h>
#include <QCryptographicHash>
#include "Server.h"
#include "HttpServer.h"
#include "Request.h"
#include "Response.h"
#include "ResponseEntry.h"
#include "crypto/Crypto.h"

using namespace KeepassHttpProtocol;

Server::Server(QObject* parent)
    : QObject(parent)
{ }

Server::~Server()
{
    stop();
}

void Server::start(const char* host, int port)
{
    if (m_httpServer && m_httpServer->isStarted()) {
        return;
    }

    m_httpServer.reset(new HttpServer(host, port));
    connect(m_httpServer.get(), SIGNAL(acceptRequest(evhttp_request*, bool*)),
            this, SLOT(acceptRequest(evhttp_request*, bool*)),
            Qt::BlockingQueuedConnection);
    connect(m_httpServer.get(), SIGNAL(handleRequest(const QByteArray, QByteArray*, int*)),
            this, SLOT(handleRequest(const QByteArray, QByteArray*, int*)),
            Qt::BlockingQueuedConnection);
}

void Server::stop()
{
    if (!m_httpServer) {
        return;
    }
    m_httpServer.reset();
}

bool Server::isStarted() const
{
    return m_httpServer && m_httpServer->isStarted();
}

void Server::acceptRequest(evhttp_request* request, bool* accept)
{
    *accept = request && QString(evhttp_find_header(request->input_headers, "Content-Type")).compare("application/json", Qt::CaseInsensitive) == 0;
}


void Server::handleRequest(const QByteArray request, QByteArray* response, int* status)
{
    Request r;
    if (!isDatabaseOpened() && !openDatabase()) {
        *status = HTTP_SERVUNAVAIL;
    } else if (r.fromJson(request)) {
        QByteArray hash = QCryptographicHash::hash((getDatabaseRootUuid() + getDatabaseRecycleBinUuid()).toUtf8(),
                                                   QCryptographicHash::Sha1).toHex();

        bool success = true;
        Response protocolResp(r, QString::fromLatin1(hash));
        switch(r.requestType()) {
            case INVALID:           success = false; break;
            case TEST_ASSOCIATE:    testAssociate(r, &protocolResp); break;
            case ASSOCIATE:         associate(r, &protocolResp); break;
            case GET_LOGINS:        getLogins(r, &protocolResp); break;
            case GET_LOGINS_COUNT:  getLoginsCount(r, &protocolResp); break;
            case GET_ALL_LOGINS:    getAllLogins(r, &protocolResp); break;
            case SET_LOGIN:         setLogin(r, &protocolResp); break;
            case GENERATE_PASSWORD: generatePassword(r, &protocolResp); break;
        }

        if (success) {
            *response = protocolResp.toJson().toUtf8();
            *status = HTTP_OK;
        } else {
            *status = HTTP_BADREQUEST;
        }
    } else {
        *status = HTTP_BADREQUEST;
    }
}

void Server::testAssociate(const Request& r, Response* protocolResp)
{
    if (r.id().isEmpty())
        return;  // ping

    QString key = getKey(r.id());
    if (key.isEmpty() || !r.CheckVerifier(key))
        return;

    protocolResp->setSuccess();
    protocolResp->setId(r.id());
    protocolResp->setVerifier(key);
}

void Server::associate(const Request& r, Response* protocolResp)
{
    if (!r.CheckVerifier(r.key()))
        return;

    QString id = storeKey(r.key());
    if (id.isEmpty())
        return;

    protocolResp->setSuccess();
    protocolResp->setId(id);
    protocolResp->setVerifier(r.key());
}

void Server::getLogins(const Request& r, Response* protocolResp)
{
    QString key = getKey(r.id());
    if (!r.CheckVerifier(key))
        return;

    protocolResp->setSuccess();
    protocolResp->setId(r.id());
    protocolResp->setVerifier(key);
    QList<ResponseEntry> entries = findMatchingEntries(r.id(), r.url(), r.submitUrl(), r.realm());  // TODO: filtering, request confirmation [in db adaptation layer?]
    if (r.sortSelection()) {
        // TODO: sorting (in db adaptation layer? here?)
    }
    protocolResp->setEntries(entries);
}

void Server::getLoginsCount(const Request& r, Response* protocolResp)
{
    QString key = getKey(r.id());
    if (!r.CheckVerifier(key))
        return;

    protocolResp->setSuccess();
    protocolResp->setId(r.id());
    protocolResp->setVerifier(key);
    protocolResp->setCount(countMatchingEntries(r.id(), r.url(), r.submitUrl(), r.realm()));
}

void Server::getAllLogins(const Request& r, Response* protocolResp)
{
    QString key = getKey(r.id());
    if (!r.CheckVerifier(key))
        return;

    protocolResp->setSuccess();
    protocolResp->setId(r.id());
    protocolResp->setVerifier(key);
    protocolResp->setEntries(searchAllEntries(r.id()));  // TODO: ensure there is no password --> change API?
}

void Server::setLogin(const Request& r, Response* protocolResp)
{
    QString key = getKey(r.id());
    if (!r.CheckVerifier(key))
        return;

    QString uuid = r.uuid();
    if (uuid.isEmpty())
        addEntry(r.id(), r.login(), r.password(), r.url(), r.submitUrl(), r.realm());
    else
        updateEntry(r.id(), r.uuid(), r.login(), r.password(), r.url());

    protocolResp->setSuccess();
    protocolResp->setId(r.id());
    protocolResp->setVerifier(key);
}

void Server::generatePassword(const Request& r, Response* protocolResp)
{
    QString key = getKey(r.id());
    if (!r.CheckVerifier(key))
        return;

    // TODO: actually generate password
    QString password = "0000";
    QString bits = "3";

    protocolResp->setSuccess();
    protocolResp->setId(r.id());
    protocolResp->setVerifier(key);
    protocolResp->setEntries(QList<ResponseEntry>() << ResponseEntry("generate-password", bits, password, "generate-password"));

    memset(password.data(), 0, password.length());
}
