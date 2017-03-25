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

#include "TestServer.h"

#include <evhttp.h>
#include <QObject>
#include <QTest>
#include <QMap>
#include <QList>
#include <QUrl>
#include <QtConcurrent>
#include "crypto/Crypto.h"
#include "crypto/SymmetricCipherGcrypt.h"
#include "core/Uuid.h"
#include "http/Protocol.h"
#include "http/Server.h"
#include "http/ResponseEntry.h"
#include "http/Request.h"
#include "HttpUtil.h"

using namespace KeepassHttpProtocol;

QTEST_GUILESS_MAIN(TestServer)

static QString testId = "000000000000000000";
static QByteArray testKey = QByteArray::fromHex("603deb1015ca71be2b73aef0857d77811f352c073b6108d72d9810a30914dff4");
static QByteArray testIv = QByteArray::fromHex("000102030405060708090a0b0c0d0e0f");
static QString testUrl = "https://example.com";
static QString testName = "example.com";
static QString testLogin = "test@example.com";
static QString testPassword = "password1";
static QString testUuid = "36a03c232a2c4987ac608ada66f58fcc";

MockServer::MockServer(QObject* parent)
    : Server(parent)
    , m_isDatabaseOpened(true)
    , m_isDatabaseLocked(false)
{
    m_keys[testId] = encode64(testKey);
    m_responeEntries[testId].append(ResponseEntry(testName, testLogin, testPassword, testUuid));
}

bool MockServer::isDatabaseOpened() const
{
    return m_isDatabaseOpened;
}

bool MockServer::openDatabase()
{
    if (!m_isDatabaseLocked) {
        m_isDatabaseOpened = true;
        return !m_isDatabaseLocked;
    } else {
        return false;
    }
}

QString MockServer::getDatabaseRootUuid()
{
    return QString("5c422b802f274aef94f649cd2e171f45");
}

QString MockServer::getDatabaseRecycleBinUuid()
{
    return QString("f949057f750142fa9d1cb657fb1824ca");
}

QString MockServer::getKey(const QString& id)
{
    return m_keys[id];
}

QString MockServer::storeKey(const QString& key)
{
    QString id = Uuid::random().toHex();
    m_keys[id] = key;
    return id;
}

QList<ResponseEntry> MockServer::findMatchingEntries(const QString& id, const QString&, const QString&, const QString&)
{
    return m_responeEntries[id];
}

int MockServer::countMatchingEntries(const QString& id, const QString&, const QString&, const QString&)
{
    return m_responeEntries[id].count();
}

QList<ResponseEntry> MockServer::searchAllEntries(const QString& id)
{
    QList<ResponseEntry> result;
    const QList<ResponseEntry>& entries = m_responeEntries[id];
    for (const ResponseEntry& entry : entries) {
        result.append(ResponseEntry(entry.name(), entry.login(), QString(), entry.uuid()));
    }
    return result;
}

void MockServer::addEntry(const QString& id, const QString& login, const QString& password, const QString& url, const QString&, const QString&)
{
    m_responeEntries[id].append(ResponseEntry(QUrl(url).host(), login, password, Uuid::random().toHex()));
}

void MockServer::updateEntry(const QString& id, const QString& uuid, const QString& login, const QString& password, const QString&)
{
    QList<ResponseEntry>& lst = m_responeEntries[id];
    for (int i = 0; i < lst.count(); ++i) {
        const ResponseEntry& entry = lst[i];
        if (entry.uuid() == uuid) {
            lst[i] = ResponseEntry(entry.name(), login, password, uuid);
            break;
        }
    }
}

void MockServer::setIsDatabaseOpened(bool opened)
{
    m_isDatabaseOpened = opened;
}

void MockServer::setIsDatabaseLocked(bool locked)
{
    m_isDatabaseLocked = locked;
}

QJsonObject TestServer::getRequest(const QString& requestType, const QString& id = testId, const QByteArray& key = testKey)
{
    return createRequest(requestType, id, key, testIv);
}

void TestServer::initTestCase()
{
    QVERIFY(Crypto::init());

    m_localHost = "127.0.0.1";
    m_port = 19458;
    m_mockServer = new MockServer(this);
    m_mockServer->start(m_localHost, m_port);

    QVERIFY(m_mockServer->isStarted());
}

void TestServer::testTestAssociate()
{
    QJsonObject request;
    QJsonObject response;

    // test invalid request type
    request.insert("RequestType", QString("xxx"));
    QCOMPARE(sendRequest(request, &response), HTTP_BADREQUEST);

    // test empty id
    request.insert("RequestType", QString(STR_TEST_ASSOCIATE));
    QCOMPARE(sendRequest(request, &response), HTTP_OK);
    QVERIFY(!response["Success"].toBool());

    // test non-existing id
    request = getRequest(QString(STR_TEST_ASSOCIATE), testId + "x");
    QCOMPARE(sendRequest(request, &response), HTTP_OK);
    QVERIFY(!response["Success"].toBool());

    // test existing id
    request = getRequest(QString(STR_TEST_ASSOCIATE));

    QCOMPARE(sendRequest(request, &response), HTTP_OK);
    QVERIFY(response["Success"].toBool());

    checkVerifier(response, testId, testKey);

    // test locked db
    m_mockServer->setIsDatabaseOpened(false);
    m_mockServer->setIsDatabaseLocked(true);

    request = getRequest(QString(STR_TEST_ASSOCIATE));

    QCOMPARE(sendRequest(request, &response), HTTP_SERVUNAVAIL);
    QVERIFY(!response["Success"].toBool());

    // test open db
    m_mockServer->setIsDatabaseOpened(false);
    m_mockServer->setIsDatabaseLocked(false);

    request = getRequest(QString(STR_TEST_ASSOCIATE));

    QCOMPARE(sendRequest(request, &response), HTTP_OK);
    QVERIFY(response["Success"].toBool());

    checkVerifier(response, testId, testKey);
}

void TestServer::testAssociate()
{
    QJsonObject request;
    QJsonObject response;

    QByteArray xKey = QByteArray::fromHex("602eec1015ca71be2b73aef0857d77811f352c073b6108d72d9810a30914dff4");
    // associate with the new id
    request = getRequest(QString(STR_ASSOCIATE), QString(), xKey);
    QCOMPARE(sendRequest(request, &response), HTTP_OK);
    QVERIFY(response["Success"].toBool());

    QString xId = response["Id"].toString();
    checkVerifier(response, xId, xKey);

    // test existing id
    request = getRequest(QString(STR_TEST_ASSOCIATE), xId, xKey);

    QCOMPARE(sendRequest(request, &response), HTTP_OK);
    QVERIFY(response["Success"].toBool());

    checkVerifier(response, xId, xKey);
}

void TestServer::testGetLogins()
{
    QJsonObject request;
    QJsonObject response;

    request = getRequest(QString(STR_GET_LOGINS));
    QCOMPARE(sendRequest(request, &response), HTTP_OK);
    QVERIFY(response["Success"].toBool());
    checkVerifier(response, testId, testKey);

    QJsonArray resEntries = response["Entries"].toArray();
    QCOMPARE(resEntries.size(), 1);

    QString nonce = response["Nonce"].toString();
    SymmetricCipherGcrypt cipher(SymmetricCipher::Aes256, SymmetricCipher::Cbc, SymmetricCipher::Decrypt);
    cipher.init();
    cipher.setKey(testKey);
    cipher.setIv(decode64(nonce));

    QJsonObject entry = resEntries.first().toObject();
    QCOMPARE(decrypt(entry["Name"].toString(), cipher), testName);
    QCOMPARE(decrypt(entry["Login"].toString(), cipher), testLogin);
    QCOMPARE(decrypt(entry["Password"].toString(), cipher), testPassword);
    QCOMPARE(decrypt(entry["Uuid"].toString(), cipher), testUuid);
}

void TestServer::testGetLoginsCount()
{
    QJsonObject request;
    QJsonObject response;

    request = getRequest(QString(STR_GET_LOGINS_COUNT));
    QCOMPARE(sendRequest(request, &response), HTTP_OK);
    QVERIFY(response["Success"].toBool());
    checkVerifier(response, testId, testKey);

    QCOMPARE(response["Count"].toInt(), 1);
}

void TestServer::testGetAllLogins()
{
    QJsonObject request;
    QJsonObject response;

    request = getRequest(QString(STR_GET_ALL_LOGINS));
    QCOMPARE(sendRequest(request, &response), HTTP_OK);
    QVERIFY(response["Success"].toBool());
    checkVerifier(response, testId, testKey);

    QJsonArray resEntries = response["Entries"].toArray();
    QCOMPARE(resEntries.size(), 1);

    QString nonce = response["Nonce"].toString();
    SymmetricCipherGcrypt cipher(SymmetricCipher::Aes256, SymmetricCipher::Cbc, SymmetricCipher::Decrypt);
    cipher.init();
    cipher.setKey(testKey);
    cipher.setIv(decode64(nonce));

    QJsonObject entry = resEntries.first().toObject();
    QCOMPARE(decrypt(entry["Name"].toString(), cipher), testName);
    QCOMPARE(decrypt(entry["Login"].toString(), cipher), testLogin);
    QCOMPARE(decrypt(entry["Password"].toString(), cipher), QString());
    QCOMPARE(decrypt(entry["Uuid"].toString(), cipher), testUuid);
}

void TestServer::testSetLogin()
{
    QJsonObject request;
    QJsonObject response;

    // associate with the new id
    request = getRequest(QString(STR_ASSOCIATE));
    QCOMPARE(sendRequest(request, &response), HTTP_OK);
    QVERIFY(response["Success"].toBool());

    QString xId = response["Id"].toString();
    checkVerifier(response, xId, testKey);

    // add new login
    SymmetricCipherGcrypt cipher(SymmetricCipher::Aes256, SymmetricCipher::Cbc, SymmetricCipher::Encrypt);
    cipher.init();
    cipher.setKey(testKey);
    cipher.setIv(testIv);

    QString xUrl = testUrl + ".us";
    QString xName = testName + ".us";
    QString xLogin = testLogin + ".us";
    QString xPassword = testPassword + ".us";

    request = getRequest(QString(STR_SET_LOGIN), xId);
    request["Url"] = encrypt(xUrl, cipher);
    request["Login"] = encrypt(xLogin, cipher);
    request["Password"] = encrypt(xPassword, cipher);
    QCOMPARE(sendRequest(request, &response), HTTP_OK);
    QVERIFY(response["Success"].toBool());
    checkVerifier(response, xId, testKey);

    // check login
    request = getRequest(QString(STR_GET_LOGINS), xId);
    QCOMPARE(sendRequest(request, &response), HTTP_OK);
    QVERIFY(response["Success"].toBool());
    checkVerifier(response, xId, testKey);

    QJsonArray resEntries = response["Entries"].toArray();
    QCOMPARE(resEntries.size(), 1);

    QString nonce = response["Nonce"].toString();
    SymmetricCipherGcrypt dcipher(SymmetricCipher::Aes256, SymmetricCipher::Cbc, SymmetricCipher::Decrypt);
    dcipher.init();
    dcipher.setKey(testKey);
    dcipher.setIv(decode64(nonce));

    QJsonObject entry = resEntries.first().toObject();
    QCOMPARE(decrypt(entry["Name"].toString(), dcipher), xName);
    QCOMPARE(decrypt(entry["Login"].toString(), dcipher), xLogin);
    QCOMPARE(decrypt(entry["Password"].toString(), dcipher), xPassword);

    QString xUuid = decrypt(entry["Uuid"].toString(), dcipher);

    // modify login
    QString yUrl = testUrl + ".de";
    QString yLogin = testLogin + ".de";
    QString yPassword = testPassword + ".de";

    request = getRequest(QString(STR_SET_LOGIN), xId);
    request["Url"] = encrypt(yUrl, cipher);
    request["Login"] = encrypt(yLogin, cipher);
    request["Password"] = encrypt(yPassword, cipher);
    request["Uuid"] = encrypt(xUuid, cipher);
    QCOMPARE(sendRequest(request, &response), HTTP_OK);
    QVERIFY(response["Success"].toBool());
    checkVerifier(response, xId, testKey);

    // check login
    request = getRequest(QString(STR_GET_LOGINS), xId);
    QCOMPARE(sendRequest(request, &response), HTTP_OK);
    QVERIFY(response["Success"].toBool());
    checkVerifier(response, xId, testKey);

    resEntries = response["Entries"].toArray();
    QCOMPARE(resEntries.size(), 1);

    nonce = response["Nonce"].toString();
    dcipher.setKey(testKey);
    dcipher.setIv(decode64(nonce));

    entry = resEntries.first().toObject();
    QCOMPARE(decrypt(entry["Name"].toString(), dcipher), xName);
    QCOMPARE(decrypt(entry["Login"].toString(), dcipher), yLogin);
    QCOMPARE(decrypt(entry["Password"].toString(), dcipher), yPassword);
    QCOMPARE(decrypt(entry["Uuid"].toString(), dcipher), xUuid);
}

void TestServer::testGeneratePassword()
{
    QJsonObject request;
    QJsonObject response;

    request = getRequest(QString(STR_GENERATE_PASSWORD));
    QCOMPARE(sendRequest(request, &response), HTTP_OK);
    QVERIFY(response["Success"].toBool());
    checkVerifier(response, testId, testKey);

    QJsonDocument doc(response);
    QString payload = doc.toJson(QJsonDocument::Compact);

    QJsonArray resEntries = response["Entries"].toArray();
    QCOMPARE(resEntries.size(), 1);

    QString nonce = response["Nonce"].toString();
    SymmetricCipherGcrypt cipher(SymmetricCipher::Aes256, SymmetricCipher::Cbc, SymmetricCipher::Decrypt);
    cipher.init();
    cipher.setKey(testKey);
    cipher.setIv(decode64(nonce));

    QJsonObject entry = resEntries.first().toObject();
    QCOMPARE(decrypt(entry["Name"].toString(), cipher), QString(STR_GENERATE_PASSWORD));
    QVERIFY(decrypt(entry["Login"].toString(), cipher).toInt() >= 0);
    QVERIFY(decrypt(entry["Password"].toString(), cipher).size() > 0);
    QCOMPARE(decrypt(entry["Uuid"].toString(), cipher), QString(STR_GENERATE_PASSWORD));
}

void TestServer::cleanupTestCase()
{
    m_mockServer->stop();
}

int TestServer::sendRequest(const QJsonObject& request, QJsonObject* response)
{
    return sendHttpRequest(m_localHost, m_port, request, response);
}
