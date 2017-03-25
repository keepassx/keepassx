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

#include "TestResponse.h"

#include <QTest>
#include <QJsonObject>
#include "crypto/Crypto.h"
#include "crypto/SymmetricCipherGcrypt.h"
#include "http/Protocol.h"
#include "http/Response.h"

QTEST_GUILESS_MAIN(TestResponse)

using namespace KeepassHttpProtocol;

void TestResponse::initTestCase()
{
    QVERIFY(Crypto::init());
}

void TestResponse::testEncrypt()
{
    QJsonObject reqJson;
    reqJson.insert("RequestType", QString(STR_GET_ALL_LOGINS));
    QJsonDocument doc(reqJson);
    QString strJson(doc.toJson(QJsonDocument::Compact));
    Request req;
    QVERIFY(req.fromJson(strJson));

    QByteArray hash = QCryptographicHash::hash("00000000000000000000", QCryptographicHash::Sha1).toHex();
    QByteArray key = QByteArray::fromHex("603deb1015ca71be2b73aef0857d77811f352c073b6108d72d9810a30914dff4");

    Response protocolResp(req, QString::fromLatin1(hash));
    protocolResp.setSuccess();
    protocolResp.setId("999999999999");
    protocolResp.setVerifier(encode64(key));

    QMap<QString, QMap<QString, QString>> logins;

    QString name1 = "example.com";
    QString login1 = "test@example.com";
    QString password1 = "password1";
    QString uuid1 = QByteArray::fromHex("36a03c232a2c4987ac608ada66f58fcc");

    QMap<QString, QString> loginItem1;
    loginItem1["Name"] = name1;
    loginItem1["Login"] = login1;
    loginItem1["Password"] = password1;
    loginItem1["Uuid"] = uuid1;
    logins[uuid1] = loginItem1;

    QString name2 = "test.com";
    QString login2 = "test@test.com";
    QString password2 = "password2";
    QString uuid2 = QByteArray::fromHex("f60b1e245d7d4b8582cad4c004b99f35");

    QMap<QString, QString> loginItem2;
    loginItem2["Name"] = name2;
    loginItem2["Login"] = login2;
    loginItem2["Password"] = password2;
    loginItem2["Uuid"] = uuid2;
    logins[uuid2] = loginItem2;

    QList<ResponseEntry> entries;
    entries.append(ResponseEntry(name1, login1, password1, uuid1)); 
    entries.append(ResponseEntry(name2, login2, password2, uuid2)); 

    protocolResp.setEntries(entries);

    QString resJson = protocolResp.toJson();

    QVariantMap map = QJsonDocument::fromJson(resJson.toUtf8()).object().toVariantMap();
    QCOMPARE(map.value("RequestType").toString().toUtf8(), QByteArray(STR_GET_ALL_LOGINS));
    QCOMPARE(map.value("Success").toBool(), true);
    QCOMPARE(map.value("Hash").toString().toUtf8(), hash);
    QCOMPARE(map.value("Count").toInt(), entries.count());

    QString nonce = map.value("Nonce").toString();
    QString verifier = map.value("Verifier").toString();
    SymmetricCipherGcrypt cipher(SymmetricCipher::Aes256, SymmetricCipher::Cbc, SymmetricCipher::Decrypt);
    cipher.init();
    cipher.setKey(key);
    cipher.setIv(decode64(nonce));

    QList<QVariant> resEntries = map.value("Entries").toList();
    QCOMPARE(resEntries.count(), entries.count());

    for (QList<QVariant>::iterator iter = resEntries.begin(); iter != resEntries.end(); ++iter) {
        QVariantMap entryMap = iter->toMap();
        QString name = decrypt(entryMap.value("Name").toString(), cipher);
        QString login = decrypt(entryMap.value("Login").toString(), cipher);
        QString password = decrypt(entryMap.value("Password").toString(), cipher);
        QString uuid = decrypt(entryMap.value("Uuid").toString(), cipher);
        QCOMPARE(name, logins[uuid]["Name"]);
        QCOMPARE(login, logins[uuid]["Login"]);
        QCOMPARE(password, logins[uuid]["Password"]);
        QCOMPARE(uuid, logins[uuid]["Uuid"]);
    }
}
