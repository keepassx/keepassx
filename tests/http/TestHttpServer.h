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

#ifndef KEEPASSX_TESTHTTPSERVER_H
#define KEEPASSX_TESTHTTPSERVER_H

#include <evhttp.h>
#include <QObject>
#include <http/HttpServer.h>

class TestHttpServer : public QObject
{
    Q_OBJECT

private Q_SLOTS:
    void initTestCase();
    void testRejectRequest();
    void testAcceptRequest();
    void testHandleRequestError();
    void testHandleRequestSuccess();
    void cleanupTestCase();
    void acceptRequest(evhttp_request* request, bool* accept);
    void rejectRequest(evhttp_request* request, bool* accept);
    void handleRequest(const QByteArray request, QByteArray* response, int* status);

private:
    const char* m_localHost;
    int m_port;
    KeepassHttpProtocol::HttpServer* m_httpServer;
};

#endif // KEEPASSX_TESTHTTPSERVER_H
