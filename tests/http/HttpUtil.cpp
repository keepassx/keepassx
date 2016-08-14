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
#include "http/HttpUtil.h"

#include <evhttp.h>
#include <QTest>
#include <QtConcurrent>
#include "crypto/Crypto.h"
#include "crypto/SymmetricCipherGcrypt.h"
#include "http/Protocol.h"

using namespace KeepassHttpProtocol;

struct CommunicationMessage {
    event_base* base;
    int* status;
    QByteArray* response;
};

void onCompleted(evhttp_request* req, void* arg)
{
    CommunicationMessage* msg = static_cast<CommunicationMessage*>(arg);
    if (req) {
        *msg->status = req->response_code;
        struct evbuffer* buf = evhttp_request_get_input_buffer(req);
        size_t size = evbuffer_get_length(buf);
        char* data = static_cast<char*>(malloc(sizeof(char) * size));
        memset(data, 0, size);
        evbuffer_copyout(buf, data, size);
        *msg->response = QByteArray(data, size);
        free(data);
    } else {
        *msg->status = HTTP_SERVUNAVAIL;
    }
    event_base_loopbreak(msg->base);
}

QFuture<void> makeRequest(const char* address, int port, const QByteArray request, QByteArray* response, int* status)
{
    void (*dispatch)(const char*, int, const QByteArray, QByteArray*, int*) = [] (const char* address, int port, const QByteArray request, QByteArray* response, int* status)
    {
        struct event_base* base = event_base_new();
        if (base) {
            struct CommunicationMessage msg{base, status, response};
            evhttp_request* req = evhttp_request_new(&onCompleted, &msg);
            if (req) {
                evhttp_connection* conn = evhttp_connection_base_new(base, nullptr, address, port);
                if (conn) {
                    evhttp_add_header(evhttp_request_get_output_headers(req), "Host", "localhost");
                    evhttp_add_header(evhttp_request_get_output_headers(req), "Content-Type", "application/json");
                    evbuffer_add_printf(evhttp_request_get_output_buffer(req), "%s", request.constData());
                    evhttp_make_request(conn, req, EVHTTP_REQ_POST, "/");
                    event_base_dispatch(base);
                    evhttp_connection_free(conn);
                }
            }
            event_base_free(base);
        }
    };
    return QtConcurrent::run(dispatch, address, port, request, response, status);
}

void checkVerifier(const QJsonObject& response, const QString& id, const QByteArray& key)
{
    QString nonce = response["Nonce"].toString();
    SymmetricCipherGcrypt cipher(SymmetricCipher::Aes256, SymmetricCipher::Cbc, SymmetricCipher::Decrypt);
    cipher.init();
    cipher.setKey(key);
    cipher.setIv(decode64(nonce));

    QCOMPARE(decrypt(response["Verifier"].toString(), cipher), nonce);
    QCOMPARE(response["Id"].toString(), id);
}

QJsonObject createRequest(const QString& requestType, const QString& id, const QByteArray& key, const QByteArray& iv)
{
    QJsonObject request;
    SymmetricCipherGcrypt cipher(SymmetricCipher::Aes256, SymmetricCipher::Cbc, SymmetricCipher::Encrypt);
    cipher.init();
    cipher.setKey(key);
    cipher.setIv(iv);
    QString nonce = encode64(iv);

    request.insert("RequestType", requestType);
    request.insert("Id", id);
    request.insert("Nonce", nonce);
    request.insert("Verifier", encrypt(nonce, cipher));

    if (requestType == QString(STR_ASSOCIATE)) {
        request.insert("Key", encode64(key));
    }

    return request;
}

QFuture<void> sendHttpRequestAsync(const char* host, int port, const QJsonObject& request, QByteArray* response, int* status)
{
    QJsonDocument doc(request);
    QByteArray payload = doc.toJson(QJsonDocument::Compact);

    return makeRequest(host, port, payload, response, status);
}

int sendHttpRequest(const char* host, int port, const QJsonObject& request, QJsonObject* response)
{
    int status = 0;
    QByteArray res;

    QFuture<void> future = sendHttpRequestAsync(host, port, request, &res, &status);

    while (future.isRunning()) {
        QTest::qWait(20);
    }

    if (status == HTTP_OK) {
        *response = QJsonDocument::fromJson(res).object();
    } else {
        *response = QJsonObject();
    }
    return status;
}
