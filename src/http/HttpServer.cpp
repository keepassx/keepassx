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

#include <evhttp.h>
#include <QtConcurrent>
#include "core/Tools.h"
#include "HttpServer.h"

using namespace KeepassHttpProtocol;

HttpServer::HttpServer(const char* localhost, int port)
    : m_eventBase(event_base_new(), &event_base_free)
    , m_stopped(false)
    , m_started(false)
{
    if (m_eventBase) {
        void (*dispatch)(HttpServer*, const char*, int) = [] (HttpServer* server, const char* localhost, int port)
        {
            std::unique_ptr<evhttp, decltype(&evhttp_free)> httpServer(evhttp_new(server->m_eventBase.get()), &evhttp_free);
            if (httpServer) {
                int ret = evhttp_bind_socket(httpServer.get(), localhost, port);
                if (ret == 0) {
                    void (*handler)(evhttp_request* req, void*) = [] (evhttp_request* req, void* ptr)
                    {
                        HttpServer *srv = static_cast<HttpServer *>(ptr);
                        srv->receiveRequest(req);
                        event_base_loopbreak(srv->m_eventBase.get());
                    };
                    evhttp_set_gencb(httpServer.get(), handler, server);
                    server->m_started = true;
                    while (!server->m_stopped) {
                      struct timeval waitTime;
                      waitTime.tv_sec = 1;
                      waitTime.tv_usec = 0;
                      event_base_loopexit(server->m_eventBase.get(), &waitTime);
                      event_base_dispatch(server->m_eventBase.get());
                    }
                }
            }
            server->m_started = true;
        };

        m_loop = QtConcurrent::run(dispatch, this, localhost, port);
    }
}

HttpServer::~HttpServer()
{
    stop();
}

bool HttpServer::isStarted() const
{
    while (!m_started) {
        Tools::wait(100);
    }
    return m_loop.isRunning();
}

void HttpServer::stop()
{
    m_stopped = true;
    m_loop.waitForFinished();
}

void HttpServer::receiveRequest(evhttp_request* request)
{
    bool accepted = false;
    int status = HTTP_BADREQUEST;

    if (request) {
        Q_EMIT acceptRequest(request, &accepted);
    }

    if (accepted) {
        struct evbuffer* buf = evhttp_request_get_input_buffer(request);
        size_t size = evbuffer_get_length(buf);
        char* data = static_cast<char*>(malloc(sizeof(char) * size));
        memset(data, 0, size);
        evbuffer_copyout(buf, data, size);
        QByteArray qbytes = QByteArray(data, size);
        free(data);

        QByteArray s;
        Q_EMIT handleRequest(qbytes, &s, &status);

        if (status == HTTP_OK) {
            auto* outBuf = evhttp_request_get_output_buffer(request);
            if (outBuf) {
                evbuffer_add_printf(outBuf, "%s", s.constData());
                evhttp_add_header(request->output_headers, "Content-Type", "application/json");
                evhttp_send_reply(request, HTTP_OK, "", outBuf);
            } else {
                status = 500;
            }
        }
    }

    if (status != HTTP_OK) {
        evhttp_send_error(request, status, nullptr);
    }
}
