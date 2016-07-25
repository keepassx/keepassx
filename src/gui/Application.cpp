/*
 *  Copyright (C) 2012 Tobias Tangemann
 *  Copyright (C) 2012 Felix Geyer <debfx@fobos.de>
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

#include "Application.h"

#include <QAbstractNativeEventFilter>
#include <QFileOpenEvent>
#include <QLockFile>
#include <QStandardPaths>
#include <QtNetwork/QLocalSocket>

#include "autotype/AutoType.h"

#if defined(Q_OS_UNIX) && !defined(Q_OS_OSX)
class XcbEventFilter : public QAbstractNativeEventFilter
{
public:
    bool nativeEventFilter(const QByteArray& eventType, void* message, long* result) override
    {
        Q_UNUSED(result)

        if (eventType == QByteArrayLiteral("xcb_generic_event_t")) {
            int retCode = autoType()->callEventFilter(message);
            if (retCode == 1) {
                return true;
            }
        }

        return false;
    }
};
#endif

Application::Application(int& argc, char** argv)
    : QApplication(argc, argv)
    , m_mainWindow(nullptr)
    , alreadyRunning(false)
    , lock(nullptr)
{
#if defined(Q_OS_UNIX) && !defined(Q_OS_OSX)
    installNativeEventFilter(new XcbEventFilter());
#endif

    QString userName = qgetenv("USER");
    if (userName.isEmpty()) {
        userName = qgetenv("USERNAME");
    }
    QString identifier = "keepassx2";
    if (!userName.isEmpty()) {
        identifier.append("-");
        identifier.append(userName);
    }
    QString socketName = identifier + ".socket";
    QString lockName = identifier + ".lock";

    // According to documentation we should use RuntimeLocation on *nixes, but even Qt doesn't respect
    // this and creates sockets in TempLocation, so let's be consistent.
    lock = new QLockFile(QStandardPaths::writableLocation(QStandardPaths::TempLocation) + "/" + lockName);
    lock->setStaleLockTime(0);
    lock->tryLock();
    switch (lock->error()) {
    case QLockFile::NoError:
        server.setSocketOptions(QLocalServer::UserAccessOption);
        server.listen(socketName);
        connect(&server, SIGNAL(newConnection()), this, SIGNAL(anotherInstanceStarted()));
        break;
    case QLockFile::LockFailedError: {
        alreadyRunning = true;
        // notify the other instance
        // try several times, in case the other instance is still starting up
        QLocalSocket client;
        for (int i = 0; i < 3; i++) {
            client.connectToServer(socketName);
            if (client.waitForConnected(150)) {
                client.abort();
                break;
            }
        }
        break;
    }
    default:
        qWarning() << QCoreApplication::translate("Main",
                                                  "The lock file could not be created. Single-instance mode disabled.")
                      .toUtf8().constData();
    }
}

Application::~Application()
{
    server.close();
    if (lock) {
        lock->unlock();
        delete lock;
    }
}

void Application::setMainWindow(QWidget* mainWindow)
{
    m_mainWindow = mainWindow;
}

bool Application::event(QEvent* event)
{
    // Handle Apple QFileOpenEvent from finder (double click on .kdbx file)
    if (event->type() == QEvent::FileOpen) {
        Q_EMIT openFile(static_cast<QFileOpenEvent*>(event)->file());
        return true;
    }
#ifdef Q_OS_MAC
    // restore main window when clicking on the docker icon
    else if ((event->type() == QEvent::ApplicationActivate) && m_mainWindow) {
        m_mainWindow->ensurePolished();
        m_mainWindow->setWindowState(m_mainWindow->windowState() & ~Qt::WindowMinimized);
        m_mainWindow->show();
        m_mainWindow->raise();
        m_mainWindow->activateWindow();
    }
#endif

    return QApplication::event(event);
}

bool Application::isAlreadyRunning() const
{
    return alreadyRunning;
}
