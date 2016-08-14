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

#include "TestHttpServer.h"

#include <QTest>
#include <QtConcurrent>
#include <evhttp.h>

#include "http/HttpServer.h"
#include "HttpUtil.h"

QTEST_GUILESS_MAIN(TestHttpServer)

using namespace KeepassHttpProtocol;

void TestHttpServer::initTestCase()
{
    m_localHost = "127.0.0.1";
    m_port = 19458;
    m_httpServer = new HttpServer(m_localHost, m_port);
    QVERIFY(m_httpServer->isStarted());
}

void TestHttpServer::testRejectRequest()
{
    m_httpServer->disconnect();
    QVERIFY(connect(m_httpServer, SIGNAL(acceptRequest(evhttp_request*, bool*)),
            this, SLOT(rejectRequest(evhttp_request*, bool*)), Qt::BlockingQueuedConnection));
    QVERIFY(connect(m_httpServer, SIGNAL(handleRequest(const QByteArray, QByteArray*, int*)),
            this, SLOT(handleRequest(const QByteArray, QByteArray*, int*)),
            Qt::BlockingQueuedConnection));

    QByteArray request, response;
    int status = 0;
    QFuture<void> future = makeRequest(m_localHost, m_port, request, &response, &status);
    while (future.isRunning()) {
        QTest::qWait(5);
    }
    QCOMPARE(status, HTTP_BADREQUEST);
}

void TestHttpServer::testAcceptRequest()
{
    m_httpServer->disconnect();
    QVERIFY(connect(m_httpServer, SIGNAL(acceptRequest(evhttp_request*, bool*)),
            this, SLOT(acceptRequest(evhttp_request*, bool*)), Qt::BlockingQueuedConnection));
    QVERIFY(connect(m_httpServer, SIGNAL(handleRequest(const QByteArray, QByteArray*, int*)),
            this, SLOT(handleRequest(const QByteArray, QByteArray*, int*)),
            Qt::BlockingQueuedConnection));

    QByteArray request, response;
    int status = 0;
    QFuture<void> future = makeRequest(m_localHost, m_port, request, &response, &status);
    while (future.isRunning()) {
        QTest::qWait(5);
    }
    QCOMPARE(status, HTTP_OK);
}

void TestHttpServer::testHandleRequestError()
{
    m_httpServer->disconnect();
    QVERIFY(connect(m_httpServer, SIGNAL(acceptRequest(evhttp_request*, bool*)),
            this, SLOT(acceptRequest(evhttp_request*, bool*)), Qt::BlockingQueuedConnection));
    QVERIFY(connect(m_httpServer, SIGNAL(handleRequest(const QByteArray, QByteArray*, int*)),
            this, SLOT(handleRequest(const QByteArray, QByteArray*, int*)),
            Qt::BlockingQueuedConnection));

    QByteArray request = QByteArray("UNAVAIL"), response;
    int status = 0;
    QFuture<void> future = makeRequest(m_localHost, m_port, request, &response, &status);
    while (future.isRunning()) {
        QTest::qWait(5);
    }
    QCOMPARE(status, HTTP_SERVUNAVAIL);
}

void TestHttpServer::testHandleRequestSuccess()
{
    m_httpServer->disconnect();
    QVERIFY(connect(m_httpServer, SIGNAL(acceptRequest(evhttp_request*, bool*)),
            this, SLOT(acceptRequest(evhttp_request*, bool*)), Qt::BlockingQueuedConnection));
    QVERIFY(connect(m_httpServer, SIGNAL(handleRequest(const QByteArray, QByteArray*, int*)),
            this, SLOT(handleRequest(const QByteArray, QByteArray*, int*)),
            Qt::BlockingQueuedConnection));

    QByteArray request = QByteArray("ACCEPT"), response;
    int status = 0;
    QFuture<void> future = makeRequest(m_localHost, m_port, request, &response, &status);
    while (future.isRunning()) {
        QTest::qWait(5);
    }
    QCOMPARE(status, HTTP_OK);
    QCOMPARE(response, request);
}

void TestHttpServer::cleanupTestCase()
{
    m_httpServer->stop();
}

void TestHttpServer::acceptRequest(evhttp_request*, bool* accept)
{
    *accept = true;
}

void TestHttpServer::rejectRequest(evhttp_request*, bool* accept)
{
    *accept = false;
}

void TestHttpServer::handleRequest(const QByteArray request, QByteArray* response, int* status)
{
    *status = 200;
    *response = request;
    if (request == QString("UNAVAIL")) {
        *status = HTTP_SERVUNAVAIL;
    }
}
