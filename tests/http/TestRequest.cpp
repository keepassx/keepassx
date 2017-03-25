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

#include "TestRequest.h"

#include <QTest>
#include <QJsonObject>
#include "crypto/Crypto.h"
#include "crypto/SymmetricCipherGcrypt.h"
#include "http/Protocol.h"
#include "http/Request.h"

QTEST_GUILESS_MAIN(TestRequest)

using namespace KeepassHttpProtocol;

void TestRequest::initTestCase()
{
    QVERIFY(Crypto::init());
}

void TestRequest::testDecrypt()
{
    QByteArray key = QByteArray::fromHex("603deb1015ca71be2b73aef0857d77811f352c073b6108d72d9810a30914dff4");
    QByteArray iv = QByteArray::fromHex("000102030405060708090a0b0c0d0e0f");
    SymmetricCipherGcrypt cipher(SymmetricCipher::Aes256, SymmetricCipher::Cbc, SymmetricCipher::Encrypt);
    cipher.init();
    cipher.setKey(key);
    cipher.setIv(iv);

    QString login = "test@example.com";
    QString password = "password";
    QString uuid = "000000000000000000";
    QString url = "http://example.com";
    QString submitUrl = "http://example.com/submit";
    QString realm = "xxxxxxxxxx";

    QString nonce = encode64(iv);
    QJsonObject reqJson;
    reqJson.insert("Nonce", nonce);
    reqJson.insert("Verifier", encrypt(nonce, cipher));
    reqJson.insert("RequestType", QString(STR_SET_LOGIN));
    reqJson.insert("Login", encrypt(login, cipher));
    reqJson.insert("Password", encrypt(password, cipher));
    reqJson.insert("Uuid", encrypt(uuid, cipher));
    reqJson.insert("Url", encrypt(url, cipher));
    reqJson.insert("SubmitUrl", encrypt(submitUrl, cipher));
    reqJson.insert("Realm", encrypt(realm, cipher));

    QJsonDocument doc(reqJson);
    QString strJson(doc.toJson(QJsonDocument::Compact));

    QString rKey = encode64(key);
    Request req;
    QVERIFY(req.fromJson(strJson));
    QVERIFY(req.CheckVerifier(rKey));
    QCOMPARE(req.login(), login);
    QCOMPARE(req.password(), password);
    QCOMPARE(req.uuid(), uuid);
    QCOMPARE(req.url(), url);
    QCOMPARE(req.submitUrl(), submitUrl);
    QCOMPARE(req.realm(), realm);
}
