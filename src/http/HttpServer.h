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

#ifndef HTTP_SERVER_H
#define HTTP_SERVER_H

#include <memory>
#include <QFuture>
#include <QtCore>
#include <evhttp.h>

namespace KeepassHttpProtocol {

class HttpServer : public QObject
{
    Q_OBJECT

public:
    HttpServer(const char*, int);
    ~HttpServer();
    bool isStarted() const;
    void stop();

Q_SIGNALS:
    void acceptRequest(evhttp_request* request, bool* accept);
    void handleRequest(const QByteArray request, QByteArray* response, int* status);

private:
    void receiveRequest(evhttp_request* request);

    QFuture<void> m_loop;
    std::unique_ptr<event_base, decltype(&event_base_free)> m_eventBase;
    bool m_stopped;
    bool m_started;
};

}  // namespace KeepassHttpProtocol

#endif // HTTP_SERVER_H
