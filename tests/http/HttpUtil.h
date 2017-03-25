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

#ifndef KEEPASSX_HTTPUTIL_H
#define KEEPASSX_HTTPUTIL_H

#include <evhttp.h>
#include <QFuture>

QFuture<void> makeRequest(const char* address, int port, const QByteArray request, QByteArray* response, int* status);
void checkVerifier(const QJsonObject& response, const QString& id, const QByteArray& key);
QJsonObject createRequest(const QString& requestType, const QString& id, const QByteArray& key, const QByteArray& iv);
QFuture<void> sendHttpRequestAsync(const char* host, int port, const QJsonObject& request, QByteArray* response, int* status);
int sendHttpRequest(const char* host, int port, const QJsonObject& request, QJsonObject* response);

#endif // KEEPASSX_HTTPUTIL_H
