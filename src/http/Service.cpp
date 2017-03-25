/*
 *  Copyright (C) 2013 Francois Ferrand <thetypz@gmail.com>
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

#include "Service.h"
#include "ResponseEntry.h"
#include "core/Config.h"
#include "core/Database.h"
#include "core/Entry.h"
#include "core/Group.h"
#include "core/EntrySearcher.h"
#include "core/Metadata.h"
#include "core/Uuid.h"
#include "core/PasswordGenerator.h"
#include <QInputDialog>
#include <QMessageBox>
#include <QDebug>

Service::Service(DatabaseTabWidget* parent)
    : Service(parent, defaultHost, defaultPort)
{ }

Service::Service(DatabaseTabWidget* parent, const char* host, int port)
    : KeepassHttpProtocol::Server(parent)
    , m_dbTabWidget(parent)
{
    if (config()->get("http/enablehttpplugin").toBool()) {
        start(host, port);
    }
}

static const Uuid KEEPASSHTTP_UUID = Uuid(QByteArray::fromHex("34697a408a5b41c09f36897d623ecb31"));
static const char KEEPASSHTTP_NAME[] = "KeePassHttp Settings";
static const char ASSOCIATE_KEY_PREFIX[] = "AES Key: ";
static const char KEEPASSHTTP_GROUP_NAME[] = "KeePassHttp Passwords";  // Group where new KeePassHttp password are stored

Entry* Service::getConfigEntry(bool create)
{
    if (DatabaseWidget* dbWidget = m_dbTabWidget->currentDatabaseWidget())
        if (Database* db = dbWidget->database()) {
            Entry* entry = db->resolveEntry(KEEPASSHTTP_UUID);
            if (!entry && create) {
                entry = new Entry();
                entry->setTitle(QLatin1String(KEEPASSHTTP_NAME));
                entry->setUuid(KEEPASSHTTP_UUID);
                entry->setAutoTypeEnabled(false);
                entry->setGroup(db->rootGroup());
            } else if (entry && entry->group() == db->metadata()->recycleBin()) {
                if (create) {
                    entry->setGroup(db->rootGroup());
                } else {
                    entry = NULL;
                }
            }
            return entry;
        }
    return NULL;
}

bool Service::isDatabaseOpened() const
{
    if (DatabaseWidget* dbWidget = m_dbTabWidget->currentDatabaseWidget()) {
        switch(dbWidget->currentMode()) {
        case DatabaseWidget::None:
        case DatabaseWidget::LockedMode:
            break;

        case DatabaseWidget::ViewMode:
        case DatabaseWidget::EditMode:
            return true;
        }
    }
    return false;
}

bool Service::openDatabase()
{
    if (DatabaseWidget* dbWidget = m_dbTabWidget->currentDatabaseWidget()) {
        if (dbWidget->currentMode() == DatabaseWidget::LockedMode) {
            // TODO: show notification
            //       open window
            //       wait a few seconds for user to unlock...
        }
    }
    return false;
}

QString Service::getDatabaseRootUuid()
{
    if (DatabaseWidget* dbWidget = m_dbTabWidget->currentDatabaseWidget()) {
        if (Database* db = dbWidget->database()) {
            if (Group* rootGroup = db->rootGroup()) {
                return rootGroup->uuid().toHex();
            }
        }
    }
    return QString();
}

QString Service::getDatabaseRecycleBinUuid()
{
    if (DatabaseWidget* dbWidget = m_dbTabWidget->currentDatabaseWidget()) {
        if (Database* db = dbWidget->database()) {
            if (Group* recycleBin = db->metadata()->recycleBin()) {
                return recycleBin->uuid().toHex();
            }
        }
    }
    return QString();
}

QString Service::getKey(const QString& id)
{
    if (Entry* config = getConfigEntry()) {
        return config->attributes()->value(QLatin1String(ASSOCIATE_KEY_PREFIX) + id);
    }
    return QString();
}

QString Service::storeKey(const QString& key)
{
    QString id;
    if (Entry* config = getConfigEntry(true)) {
        do {
            bool ok;
            // Indicate who wants to associate, and request user to enter the 'name' of association key
            id = QInputDialog::getText(0, tr("KeyPassX/Http: New key association request"),
                                       tr("You have received an association request for the above key. If you would like to "
                                          "allow it access to your KeePassX database give it a unique name to identify and a"
                                          "ccept it."),
                                       QLineEdit::Normal, QString(), &ok);
            if (!ok || id.isEmpty()) {
                return QString();
            }
            // Warn if association key already exists
        } while(!config->attributes()->value(QLatin1String(ASSOCIATE_KEY_PREFIX) + id).isEmpty() &&
            QMessageBox::warning(0, tr("KeyPassX/Http: Overwrite existing key?"),
                                 tr("A shared encryption-key with the name \"%1\" already exists.\nDo you want to overwrite it?").arg(id),
                                 QMessageBox::Yes | QMessageBox::No) == QMessageBox::No);

        config->attributes()->set(QLatin1String(ASSOCIATE_KEY_PREFIX) + id, key, true);
    }
    return id;
}

bool Service::matchUrlScheme(const QString& url)
{
    QString str = url.left(8).toLower();
    return str.startsWith("http://") ||
           str.startsWith("https://") ||
           str.startsWith("ftp://") ||
           str.startsWith("ftps://");
}

bool Service::removeFirstDomain(QString& hostname)
{
    int pos = hostname.indexOf(".");
    if (pos < 0) {
        return false;
    }
    hostname = hostname.mid(pos + 1);
    return !hostname.isEmpty();
}

QList<Entry*> Service::searchEntries(const QString& text)
{
    QList<Entry*> entries;

    // TODO: setting to search all databases [e.g. as long as the 'current' db is authentified

    // Search entries matching the hostname
    QString hostname = QUrl(text).host();
    if (DatabaseWidget* dbWidget = m_dbTabWidget->currentDatabaseWidget()) {
        if (Database* db = dbWidget->database()) {
            if (Group* rootGroup = db->rootGroup()) {
                do {
                    const QList<Entry*> matchedEntries = EntrySearcher().search(hostname, rootGroup, Qt::CaseInsensitive);
                    for (Entry* entry : matchedEntries) {
                        QString title = entry->title();
                        QString url = entry->url();

                        if (entry->uuid() != KEEPASSHTTP_UUID) {
                            // Filter to match hostname in Title and Url fields
                            if (hostname.contains(title)
                                || hostname.contains(url)
                                || (matchUrlScheme(title) && hostname.contains(QUrl(title).host()))
                                || (matchUrlScheme(url) && hostname.contains(QUrl(url).host()))) {
                                entries.append(entry);
                            }
                        }
                    }
                } while(entries.isEmpty() && removeFirstDomain(hostname));
            }
        }
    }
    return entries;
}

QList<KeepassHttpProtocol::ResponseEntry> Service::findMatchingEntries(const QString&, const QString& url, const QString&, const QString&)
{
    QList<KeepassHttpProtocol::ResponseEntry> result;
    const QList<Entry*> pwEntries = searchEntries(url);
    for (const Entry* entry : pwEntries) {
        result << KeepassHttpProtocol::ResponseEntry(entry->title(), entry->username(), entry->password(), entry->uuid().toHex());
    }
    return result;
}

int Service::countMatchingEntries(const QString&, const QString& url, const QString&, const QString&)
{
    return searchEntries(url).count();
}

QList<KeepassHttpProtocol::ResponseEntry> Service::searchAllEntries(const QString&)
{
    QList<KeepassHttpProtocol::ResponseEntry> result;
    if (DatabaseWidget* dbWidget = m_dbTabWidget->currentDatabaseWidget()) {
        if (Database* db = dbWidget->database()) {
            if (Group* rootGroup = db->rootGroup()) {
                const QList<Entry*> entries = rootGroup->entriesRecursive();
                for (const Entry* entry : entries) {
                    if (entry->uuid() != KEEPASSHTTP_UUID) {
                        result << KeepassHttpProtocol::ResponseEntry(entry->title(), entry->username(), QString(), entry->uuid().toHex());
                    }
                }
            }
        }
    }
    return result;
}

Group* Service::findCreateAddEntryGroup()
{
    if (DatabaseWidget* dbWidget = m_dbTabWidget->currentDatabaseWidget()) {
        if (Database* db = dbWidget->database()) {
            if (Group* rootGroup = db->rootGroup()) {
                const QString groupName = QLatin1String(KEEPASSHTTP_GROUP_NAME);  // TODO: setting to decide where new keys are created
                const QList<Group*> groups = rootGroup->groupsRecursive(true);
                for (const Group* g : groups) {
                    if (g->name() == groupName) {
                        return db->resolveGroup(g->uuid());
                    }
                }

                Group* group;
                group = new Group();
                group->setUuid(Uuid::random());
                group->setName(groupName);
                group->setIcon(Group::DefaultIconNumber);  // TODO: WorldIconNumber
                group->setParent(rootGroup);
                return group;
            }
        }
    }
    return NULL;
}

void Service::addEntry(const QString&, const QString& login, const QString& password, const QString& url, const QString&, const QString&)
{
    if (Group* group = findCreateAddEntryGroup()) {
        Entry* entry = new Entry();
        entry->setUuid(Uuid::random());
        entry->setTitle(QUrl(url).host());
        entry->setUrl(url);
        entry->setIcon(Entry::DefaultIconNumber);  // TODO: WorldIconNumber
        entry->setUsername(login);
        entry->setPassword(password);
        entry->setGroup(group);
    }
}

void Service::updateEntry(const QString&, const QString& uuid, const QString& login, const QString& password, const QString& url)
{
    if (DatabaseWidget* dbWidget = m_dbTabWidget->currentDatabaseWidget()) {
        if (Database* db = dbWidget->database()) {
            if (Entry* entry = db->resolveEntry(Uuid(QByteArray::fromHex(uuid.toLatin1())))) {
                QString u = entry->username();
                if (u != login || entry->password() != password) {
                    bool autoAllow = config()->get("http/autoallow").toBool();  // TODO: setting to request confirmation/auto-allow
                    if (autoAllow
                        || QMessageBox::warning(0, tr("KeyPassX/Http: Update Entry"),
                                                tr("Do you want to update the information in %1 - %2?").arg(QUrl(url).host()).arg(u),
                                                QMessageBox::Yes | QMessageBox::No) == QMessageBox::Yes) {
                        entry->beginUpdate();
                        entry->setUsername(login);
                        entry->setPassword(password);
                        entry->endUpdate();
                    }
                }
            }
        }
    }
}
