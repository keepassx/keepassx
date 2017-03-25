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

#include "TestService.h"

#include <QFuture>
#include <QAction>
#include <QApplication>
#include <QLineEdit>
#include <QTemporaryFile>
#include <QTest>

#include "config-keepassx-tests.h"
#include "core/Config.h"
#include "core/Tools.h"
#include "crypto/Crypto.h"
#include "crypto/SymmetricCipherGcrypt.h"
#include "gui/DatabaseTabWidget.h"
#include "gui/DatabaseWidget.h"
#include "gui/FileDialog.h"
#include "gui/MainWindow.h"
#include "http/Protocol.h"
#include "http/Service.h"
#include "HttpUtil.h"


using namespace KeepassHttpProtocol;

static QString testId = "sample";
static QByteArray testKey = decode64(QString("4hBlqekvJmHtwxmeeeD8TM8/Z9epnCH9BHU+1O24vrY="));
static QByteArray testIv = QByteArray::fromHex("000102030405060708090a0b0c0d0e0f");
static QString testUrl = "https://example.com";
static QString testName = "example.com";
static QString testLogin = "test@example.com";
static QString testPassword = "password1";
static QString testUuid = "c85b9437668b08effc75840bc798a1c6";

QJsonObject TestService::getRequest(const QString& requestType, const QString& id = testId, const QByteArray& key = testKey)
{
    return createRequest(requestType, id, key, testIv);
}

void TestService::initTestCase()
{
    QVERIFY(Crypto::init());
    Config::createTempFileInstance();
    config()->set("http/enablehttpplugin", true);
    config()->set("http/autoallow", true);
    m_localHost = "127.0.0.1";
    m_port = 19458;
    config()->set("http/serverhost", m_localHost);
    config()->set("http/serverport", m_port);

    m_mainWindow = new MainWindow();
    m_tabWidget = m_mainWindow->findChild<DatabaseTabWidget*>("tabWidget");
    m_mainWindow->show();
    m_mainWindow->activateWindow();
    Tools::wait(50);
    QVERIFY(m_mainWindow->isHttpServiceStarted());

    QByteArray tmpData;
    QFile sourceDbFile(QString(KEEPASSX_TEST_DATA_DIR).append("/HttpDatabase.kdbx"));
    QVERIFY(sourceDbFile.open(QIODevice::ReadOnly));
    QVERIFY(Tools::readAllFromDevice(&sourceDbFile, tmpData));

    QVERIFY(m_orgDbFile.open());
    m_orgDbFileName = QFileInfo(m_orgDbFile.fileName()).fileName();
    QCOMPARE(m_orgDbFile.write(tmpData), static_cast<qint64>((tmpData.size())));
    m_orgDbFile.close();

    // open database
    fileDialog()->setNextFileName(m_orgDbFile.fileName());
    triggerAction("actionDatabaseOpen");

    QWidget* databaseOpenWidget = m_mainWindow->findChild<QWidget*>("databaseOpenWidget");
    QLineEdit* editPassword = databaseOpenWidget->findChild<QLineEdit*>("editPassword");
    QVERIFY(editPassword);

    QTest::keyClicks(editPassword, "a");
    QTest::keyClick(editPassword, Qt::Key_Enter);

    QCOMPARE(m_tabWidget->count(), 1);
    QCOMPARE(m_tabWidget->tabText(m_tabWidget->currentIndex()), m_orgDbFileName);

    m_dbWidget = m_tabWidget->currentDatabaseWidget();
    m_db = m_dbWidget->database();
    QCOMPARE(m_dbWidget->currentMode(), DatabaseWidget::ViewMode);
}

void TestService::testTestAssociate()
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
}

void TestService::testGetLogins()
{
    QJsonObject request;
    QJsonObject response;

    SymmetricCipherGcrypt cipher(SymmetricCipher::Aes256, SymmetricCipher::Cbc, SymmetricCipher::Encrypt);
    cipher.init();
    cipher.setKey(testKey);
    cipher.setIv(testIv);

    request = getRequest(QString(STR_GET_LOGINS));
    request["Url"] = encrypt(testUrl, cipher);
    QCOMPARE(sendRequest(request, &response), HTTP_OK);
    QVERIFY(response["Success"].toBool());
    checkVerifier(response, testId, testKey);

    QJsonArray resEntries = response["Entries"].toArray();
    QCOMPARE(resEntries.size(), 1);

    QString nonce = response["Nonce"].toString();
    SymmetricCipherGcrypt dcipher(SymmetricCipher::Aes256, SymmetricCipher::Cbc, SymmetricCipher::Decrypt);
    dcipher.init();
    dcipher.setKey(testKey);
    dcipher.setIv(decode64(nonce));

    QJsonObject entry = resEntries.first().toObject();
    QCOMPARE(decrypt(entry["Name"].toString(), dcipher), testName);
    QCOMPARE(decrypt(entry["Login"].toString(), dcipher), testLogin);
    QCOMPARE(decrypt(entry["Password"].toString(), dcipher), testPassword);
    QCOMPARE(decrypt(entry["Uuid"].toString(), dcipher), testUuid);
}

void TestService::testGetLoginsCount()
{
    QJsonObject request;
    QJsonObject response;

    SymmetricCipherGcrypt cipher(SymmetricCipher::Aes256, SymmetricCipher::Cbc, SymmetricCipher::Encrypt);
    cipher.init();
    cipher.setKey(testKey);
    cipher.setIv(testIv);

    request = getRequest(QString(STR_GET_LOGINS_COUNT));
    request["Url"] = encrypt(testUrl, cipher);
    QCOMPARE(sendRequest(request, &response), HTTP_OK);
    QVERIFY(response["Success"].toBool());
    checkVerifier(response, testId, testKey);

    QCOMPARE(response["Count"].toInt(), 1);
}

void TestService::testGetAllLogins()
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

void TestService::testSetLogin()
{
    QJsonObject request;
    QJsonObject response;

    // add new login
    SymmetricCipherGcrypt cipher(SymmetricCipher::Aes256, SymmetricCipher::Cbc, SymmetricCipher::Encrypt);
    cipher.init();
    cipher.setKey(testKey);
    cipher.setIv(testIv);

    QString xUrl = testUrl + ".us";
    QString xName = testName + ".us";
    QString xLogin = testLogin + ".us";
    QString xPassword = testPassword + ".us";

    request = getRequest(QString(STR_SET_LOGIN), testId);
    request["Url"] = encrypt(xUrl, cipher);
    request["Login"] = encrypt(xLogin, cipher);
    request["Password"] = encrypt(xPassword, cipher);
    QCOMPARE(sendRequest(request, &response), HTTP_OK);
    QVERIFY(response["Success"].toBool());
    checkVerifier(response, testId, testKey);

    // check login
    request = getRequest(QString(STR_GET_LOGINS), testId);
    request["Url"] = encrypt(xUrl, cipher);
    QCOMPARE(sendRequest(request, &response), HTTP_OK);
    QVERIFY(response["Success"].toBool());
    checkVerifier(response, testId, testKey);

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

    request = getRequest(QString(STR_SET_LOGIN), testId);
    request["Url"] = encrypt(yUrl, cipher);
    request["Login"] = encrypt(yLogin, cipher);
    request["Password"] = encrypt(yPassword, cipher);
    request["Uuid"] = encrypt(xUuid, cipher);
    QCOMPARE(sendRequest(request, &response), HTTP_OK);
    QVERIFY(response["Success"].toBool());
    checkVerifier(response, testId, testKey);

    // check login
    request = getRequest(QString(STR_GET_LOGINS), testId);
    request["Url"] = encrypt(xUrl, cipher);
    QCOMPARE(sendRequest(request, &response), HTTP_OK);
    QVERIFY(response["Success"].toBool());
    checkVerifier(response, testId, testKey);

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

void TestService::cleanupTestCase()
{
    delete m_mainWindow;
}

void TestService::triggerAction(const QString& name)
{
    QAction* action = m_mainWindow->findChild<QAction*>(name);
    QVERIFY(action);
    QVERIFY(action->isEnabled());
    action->trigger();
}

int TestService::sendRequest(const QJsonObject& request, QJsonObject* response)
{
    return sendHttpRequest(m_localHost, m_port, request, response);
}

QFuture<void> TestService::sendRequestAsync(const QJsonObject& request, QByteArray* response, int* status)
{
    return sendHttpRequestAsync(m_localHost, m_port, request, response, status);
}

QTEST_MAIN(TestService)
