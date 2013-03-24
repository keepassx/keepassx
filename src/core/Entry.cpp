/*
 *  Copyright (C) 2010 Felix Geyer <debfx@fobos.de>
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

#include "Entry.h"

#include "core/Database.h"
#include "core/DatabaseIcons.h"
#include "core/Group.h"
#include "core/Metadata.h"
#include "core/Tools.h"

const int Entry::DefaultIconNumber = 0;

Entry::Entry()
    : m_attributes(new EntryAttributes(this))
    , m_attachments(new EntryAttachments(this))
    , m_autoTypeAssociations(new AutoTypeAssociations(this))
    , m_tmpHistoryItem(Q_NULLPTR)
    , m_updateTimeinfo(true)
{
    m_data.iconNumber = DefaultIconNumber;
    m_data.autoTypeEnabled = true;
    m_data.autoTypeObfuscation = 0;

    connect(m_attributes, SIGNAL(modified()), this, SIGNAL(modified()));
    connect(m_attributes, SIGNAL(defaultKeyModified()), SLOT(emitDataChanged()));
    connect(m_attachments, SIGNAL(modified()), this, SIGNAL(modified()));
    connect(m_autoTypeAssociations, SIGNAL(modified()), SIGNAL(modified()));

    connect(this, SIGNAL(modified()), SLOT(updateTimeinfo()));
    connect(this, SIGNAL(modified()), SLOT(updateModifiedSinceBegin()));
}

Entry::~Entry()
{
    if (m_group) {
        m_group->removeEntry(this);

        if (m_group->database()) {
            m_group->database()->addDeletedObject(m_uuid);
        }
    }

    qDeleteAll(m_history);
}

template <class T> inline bool Entry::set(T& property, const T& value)
{
    if (property != value) {
        property = value;
        Q_EMIT modified();
        return true;
    }
    else {
        return false;
    }
}

void Entry::updateTimeinfo()
{
    if (m_updateTimeinfo) {
        m_data.timeInfo.setLastModificationTime(Tools::currentDateTimeUtc());
        m_data.timeInfo.setLastAccessTime(Tools::currentDateTimeUtc());
    }
}

void Entry::setUpdateTimeinfo(bool value)
{
    m_updateTimeinfo = value;
}

Uuid Entry::uuid() const
{
    return m_uuid;
}

QImage Entry::icon() const
{
    if (m_data.customIcon.isNull()) {
        return databaseIcons()->icon(m_data.iconNumber);
    }
    else {
        // TODO: check if database() is 0
        return database()->metadata()->customIcon(m_data.customIcon);
    }
}

QPixmap Entry::iconPixmap() const
{
    if (m_data.customIcon.isNull()) {
        return databaseIcons()->iconPixmap(m_data.iconNumber);
    }
    else {
        QPixmap pixmap;
        if (!QPixmapCache::find(m_pixmapCacheKey, &pixmap)) {
            // TODO: check if database() is 0
            pixmap = QPixmap::fromImage(database()->metadata()->customIcon(m_data.customIcon));
            m_pixmapCacheKey = QPixmapCache::insert(pixmap);
        }

        return pixmap;
    }
}

int Entry::iconNumber() const
{
    return m_data.iconNumber;
}

Uuid Entry::iconUuid() const
{
    return m_data.customIcon;
}

QColor Entry::foregroundColor() const
{
    return m_data.foregroundColor;
}

QColor Entry::backgroundColor() const
{
    return m_data.backgroundColor;
}

QString Entry::overrideUrl() const
{
    return m_data.overrideUrl;
}

QString Entry::tags() const
{
    return m_data.tags;
}

TimeInfo Entry::timeInfo() const
{
    return m_data.timeInfo;
}

bool Entry::autoTypeEnabled() const
{
    return m_data.autoTypeEnabled;
}

int Entry::autoTypeObfuscation() const
{
    return m_data.autoTypeObfuscation;
}

QString Entry::defaultAutoTypeSequence() const
{
    return m_data.defaultAutoTypeSequence;
}

AutoTypeAssociations* Entry::autoTypeAssociations()
{
    return m_autoTypeAssociations;
}

const AutoTypeAssociations* Entry::autoTypeAssociations() const
{
    return m_autoTypeAssociations;
}

QString Entry::title() const
{
    return m_attributes->value("Title");
}

QString Entry::url() const
{
    return m_attributes->value("URL");
}

QString Entry::username() const
{
    return m_attributes->value("UserName");
}

QString Entry::password() const
{
    return m_attributes->value("Password");
}

QString Entry::notes() const
{
    return m_attributes->value("Notes");
}

bool Entry::isExpired() const
{
    return m_data.timeInfo.expires() && m_data.timeInfo.expiryTime() < Tools::currentDateTimeUtc();
}

EntryAttributes* Entry::attributes()
{
    return m_attributes;
}

const EntryAttributes* Entry::attributes() const
{
    return m_attributes;
}

EntryAttachments* Entry::attachments()
{
    return m_attachments;
}

const EntryAttachments* Entry::attachments() const
{
    return m_attachments;
}

void Entry::setUuid(const Uuid& uuid)
{
    Q_ASSERT(!uuid.isNull());
    set(m_uuid, uuid);
}

void Entry::setIcon(int iconNumber)
{
    Q_ASSERT(iconNumber >= 0);

    if (m_data.iconNumber != iconNumber || !m_data.customIcon.isNull()) {
        m_data.iconNumber = iconNumber;
        m_data.customIcon = Uuid();

        m_pixmapCacheKey = QPixmapCache::Key();

        Q_EMIT modified();
        emitDataChanged();
    }
}

void Entry::setIcon(const Uuid& uuid)
{
    Q_ASSERT(!uuid.isNull());

    if (m_data.customIcon != uuid) {
        m_data.customIcon = uuid;
        m_data.iconNumber = 0;

        m_pixmapCacheKey = QPixmapCache::Key();

        Q_EMIT modified();
        emitDataChanged();
    }
}

void Entry::setForegroundColor(const QColor& color)
{
    set(m_data.foregroundColor, color);
}

void Entry::setBackgroundColor(const QColor& color)
{
    set(m_data.backgroundColor, color);
}

void Entry::setOverrideUrl(const QString& url)
{
    set(m_data.overrideUrl, url);
}

void Entry::setTags(const QString& tags)
{
    set(m_data.tags, tags);
}

void Entry::setTimeInfo(const TimeInfo& timeInfo)
{
    m_data.timeInfo = timeInfo;
}

void Entry::setAutoTypeEnabled(bool enable)
{
    set(m_data.autoTypeEnabled, enable);
}

void Entry::setAutoTypeObfuscation(int obfuscation)
{
    set(m_data.autoTypeObfuscation, obfuscation);
}

void Entry::setDefaultAutoTypeSequence(const QString& sequence)
{
    set(m_data.defaultAutoTypeSequence, sequence);
}

void Entry::setTitle(const QString& title)
{
    m_attributes->set("Title", title, m_attributes->isProtected("Title"));
}

void Entry::setUrl(const QString& url)
{
    m_attributes->set("URL", url, m_attributes->isProtected("URL"));
}

void Entry::setUsername(const QString& username)
{
    m_attributes->set("UserName", username, m_attributes->isProtected("UserName"));
}

void Entry::setPassword(const QString& password)
{
    m_attributes->set("Password", password, m_attributes->isProtected("Password"));
}

void Entry::setNotes(const QString& notes)
{
    m_attributes->set("Notes", notes, m_attributes->isProtected("Notes"));
}

void Entry::setExpires(const bool& value)
{
    if (m_data.timeInfo.expires() != value) {
        m_data.timeInfo.setExpires(value);
        Q_EMIT modified();
    }
}

void Entry::setExpiryTime(const QDateTime& dateTime)
{
    if (m_data.timeInfo.expiryTime() != dateTime) {
        m_data.timeInfo.setExpiryTime(dateTime);
        Q_EMIT modified();
    }
}

QList<Entry*> Entry::historyItems()
{
    return m_history;
}

const QList<Entry*>& Entry::historyItems() const
{
    return m_history;
}

void Entry::addHistoryItem(Entry* entry)
{
    Q_ASSERT(!entry->parent());
    Q_ASSERT(entry->uuid() == uuid());

    m_history.append(entry);
    Q_EMIT modified();
}

void Entry::removeHistoryItems(QList<Entry*> historyEntries)
{
    bool emitModified = historyEntries.count() > 0;
    Q_FOREACH (Entry* entry, historyEntries) {
        Q_ASSERT(!entry->parent());
        Q_ASSERT(entry->uuid() == uuid());
        int numberOfRemovedEntries = m_history.removeAll(entry);
        Q_ASSERT(numberOfRemovedEntries  > 0);
        Q_UNUSED(numberOfRemovedEntries);
        delete entry;
    }

    if (emitModified) {
        Q_EMIT modified();
    }
}

void Entry::truncateHistory()
{
    const Database* db = database();

    if (!db) {
        return;
    }

    int histMaxItems = db->metadata()->historyMaxItems();
    if (histMaxItems > -1) {
        int historyCount = 0;
        QMutableListIterator<Entry*> i(m_history);
        i.toBack();
        while (i.hasPrevious()) {
            historyCount++;
            Entry* entry = i.previous();
            if (historyCount > histMaxItems) {
                delete entry;
                i.remove();
            }
        }
    }

    int histMaxSize = db->metadata()->historyMaxSize();
    if (histMaxSize > -1) {
        int size = 0;
        QSet<QByteArray> foundAttachements = attachments()->values().toSet();

        QMutableListIterator<Entry*> i(m_history);
        i.toBack();
        while (i.hasPrevious()) {
            Entry* historyItem = i.previous();

            // don't calculate size if it's already above the maximum
            if (size <= histMaxSize) {
                size += historyItem->attributes()->attributesSize();

                QSet<QByteArray> newAttachments = historyItem->attachments()->values().toSet() - foundAttachements;
                Q_FOREACH (const QByteArray& attachment, newAttachments) {
                    size += attachment.size();
                }
                foundAttachements += newAttachments;
            }

            if (size > histMaxSize) {
                delete historyItem;
                i.remove();
            }
        }
    }
}

Entry* Entry::clone() const
{
    Entry* entry = new Entry();
    entry->setUpdateTimeinfo(false);
    entry->m_uuid = m_uuid;
    entry->m_data = m_data;
    entry->m_attributes->copyDataFrom(m_attributes);
    entry->m_attachments->copyDataFrom(m_attachments);
    entry->m_autoTypeAssociations->copyDataFrom(this->m_autoTypeAssociations);
    entry->setUpdateTimeinfo(true);

    QDateTime now = Tools::currentDateTimeUtc();
    entry->m_data.timeInfo.setCreationTime(now);
    entry->m_data.timeInfo.setLastModificationTime(now);
    entry->m_data.timeInfo.setLastAccessTime(now);
    entry->m_data.timeInfo.setLocationChanged(now);

    return entry;
}

void Entry::beginUpdate()
{
    Q_ASSERT(!m_tmpHistoryItem);

    m_tmpHistoryItem = new Entry();
    m_tmpHistoryItem->setUpdateTimeinfo(false);
    m_tmpHistoryItem->m_uuid = m_uuid;
    m_tmpHistoryItem->m_data = m_data;
    m_tmpHistoryItem->m_attributes->copyDataFrom(m_attributes);
    m_tmpHistoryItem->m_attachments->copyDataFrom(m_attachments);

    m_modifiedSinceBegin = false;
}

void Entry::endUpdate()
{
    Q_ASSERT(m_tmpHistoryItem);
    if (m_modifiedSinceBegin) {
        m_tmpHistoryItem->setUpdateTimeinfo(true);
        addHistoryItem(m_tmpHistoryItem);
        truncateHistory();
    }
    else {
        delete m_tmpHistoryItem;
    }

    m_tmpHistoryItem = Q_NULLPTR;
}

void Entry::updateModifiedSinceBegin()
{
    m_modifiedSinceBegin = true;
}

Group* Entry::group()
{
    return m_group;
}

const Group* Entry::group() const
{
    return m_group;
}

void Entry::setGroup(Group* group)
{
    Q_ASSERT(group);

    if (m_group == group) {
        return;
    }

    if (m_group) {
        m_group->removeEntry(this);
        if (m_group->database() && m_group->database() != group->database()) {
            m_group->database()->addDeletedObject(m_uuid);

            // copy custom icon to the new database
            if (!iconUuid().isNull() && group->database()
                    && m_group->database()->metadata()->containsCustomIcon(iconUuid())
                    && !group->database()->metadata()->containsCustomIcon(iconUuid())) {
                group->database()->metadata()->addCustomIcon(iconUuid(), icon());
            }
        }
    }

    m_group = group;
    group->addEntry(this);

    QObject::setParent(group);

    if (m_updateTimeinfo) {
        m_data.timeInfo.setLocationChanged(Tools::currentDateTimeUtc());
    }
}

void Entry::emitDataChanged()
{
    Q_EMIT dataChanged(this);
}

const Database* Entry::database() const
{
    if (m_group) {
        return m_group->database();
    }
    else {
        return Q_NULLPTR;
    }
}

bool Entry::match(const QString& searchTerm, Qt::CaseSensitivity caseSensitivity)
{
    QStringList wordList = searchTerm.split(QRegExp("\\s"), QString::SkipEmptyParts);
    Q_FOREACH (const QString& word, wordList) {
        if (!wordMatch(word, caseSensitivity)) {
            return false;
        }
    }
    return true;
}

bool Entry::wordMatch(const QString& word, Qt::CaseSensitivity caseSensitivity)
{
    return title().contains(word, caseSensitivity) ||
            username().contains(word, caseSensitivity) ||
            url().contains(word, caseSensitivity) ||
            notes().contains(word, caseSensitivity);
}

QString Entry::resolvePlaceholders(const QString& str) const
{
    QString result = str;

    result.replace("{TITLE}", title(), Qt::CaseInsensitive);
    result.replace("{USERNAME}", username(), Qt::CaseInsensitive);
    result.replace("{URL}", url(), Qt::CaseInsensitive);
    result.replace("{PASSWORD}", password(), Qt::CaseInsensitive);
    result.replace("{NOTES}", notes(), Qt::CaseInsensitive);

    // TODO: lots of other placeholders missing

    return result;
}
