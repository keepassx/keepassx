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

#ifndef KEEPASSX_TESTSERVICE_H
#define KEEPASSX_TESTSERVICE_H

#include <QObject>
#include <QFuture>
#include <QTemporaryFile>

class Database;
class DatabaseTabWidget;
class DatabaseWidget;
class MainWindow;

class TestService : public QObject
{
    Q_OBJECT

private Q_SLOTS:
    void initTestCase();
    void testTestAssociate();
    void testGetLogins();
    void testGetLoginsCount();
    void testGetAllLogins();
    void testSetLogin();
    void cleanupTestCase();

private:
    void triggerAction(const QString& name);
    int sendRequest(const QJsonObject& request, QJsonObject* response);
    QFuture<void> sendRequestAsync(const QJsonObject& request, QByteArray* response, int* status);
    QJsonObject getRequest(const QString& requestType, const QString& id, const QByteArray& key);

    MainWindow* m_mainWindow;
    DatabaseTabWidget* m_tabWidget;
    DatabaseWidget* m_dbWidget;
    QTemporaryFile m_orgDbFile;
    QString m_orgDbFileName;
    QString m_tmpFileName;
    Database* m_db;
    const char* m_localHost;
    int m_port;
};

#endif // KEEPASSX_TESTSERVICE_H
