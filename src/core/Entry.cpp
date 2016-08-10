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

#include <QCryptographicHash>
#include <QtEndian>
#include <QDateTime>

const int Entry::DefaultIconNumber = 0;




QByteArray hmacSha1(QByteArray key, QByteArray baseString)
{
    int blockSize = 64; // HMAC-SHA-1 block size, defined in SHA-1 standard
    if (key.length() > blockSize) { // if key is longer than block size (64), reduce key length with SHA-1 compression
        key = QCryptographicHash::hash(key, QCryptographicHash::Sha1);
    }

    QByteArray innerPadding(blockSize, char(0x36)); // initialize inner padding with char "6"
    QByteArray outerPadding(blockSize, char(0x5c)); // initialize outer padding with char "\"
    // ascii characters 0x36 ("6") and 0x5c ("\") are selected because they have large
    // Hamming distance (http://en.wikipedia.org/wiki/Hamming_distance)

    for (int i = 0; i < key.length(); i++) {
        innerPadding[i] = innerPadding[i] ^ key.at(i); // XOR operation between every byte in key and innerpadding, of key length
        outerPadding[i] = outerPadding[i] ^ key.at(i); // XOR operation between every byte in key and outerpadding, of key length
    }

    // result = hash ( outerPadding CONCAT hash ( innerPadding CONCAT baseString ) ).toBase64
    QByteArray total = outerPadding;
    QByteArray part = innerPadding;
    part.append(baseString);
    total.append(QCryptographicHash::hash(part, QCryptographicHash::Sha1));
    QByteArray hashed = QCryptographicHash::hash(total, QCryptographicHash::Sha1);
    return hashed;
}



int base32_decode(const quint8 *encoded, quint8 *result, int bufSize)
{
    int buffer = 0;
    int bitsLeft = 0;
    int count = 0;

    for (const quint8 *ptr = encoded; count < bufSize && *ptr; ++ptr) {
        quint8 ch = *ptr;

        if (ch == ' ' || ch == '\t' || ch == '\r' || ch == '\n' || ch == '-') {
            continue;
        }

        buffer <<= 5;

        // Deal with commonly mistyped characters
        if (ch == '0') {
            ch = 'O';
        } else if (ch == '1') {
            ch = 'L';
        } else if (ch == '8') {
            ch = 'B';
        }

        // Look up one base32 digit
        if ((ch >= 'A' && ch <= 'Z') || (ch >= 'a' && ch <= 'z')) {
            ch = (ch & 0x1F) - 1;
        } else if (ch >= '2' && ch <= '7') {
            ch -= '2' - 26;
        } else {
            return -1;
        }

        buffer |= ch;
        bitsLeft += 5;

        if (bitsLeft >= 8) {
            result[count++] = buffer >> (bitsLeft - 8);
            bitsLeft -= 8;
        }
    }

    if (count < bufSize) {
        result[count] = '\000';
    }

    return count;
}

QString OTPgeneratePin(const QByteArray key)
{
    quint64 time = QDateTime::currentDateTime().toTime_t();
    quint64 current = qToBigEndian(time / 30);

    int secretLen = (key.length() + 7) / 8 * 5;
    quint8 secret[100];
    int res = base32_decode(reinterpret_cast<const quint8 *>(key.constData()), secret, secretLen);
    QByteArray hmac = hmacSha1(QByteArray(reinterpret_cast<const char *>(secret), res), QByteArray((char*)&current, sizeof(current)));

    int offset = (hmac[hmac.length() - 1] & 0xf);
    int binary =
            ((hmac[offset] & 0x7f) << 24)
            | ((hmac[offset + 1] & 0xff) << 16)
            | ((hmac[offset + 2] & 0xff) << 8)
            | (hmac[offset + 3] & 0xff);

    int password = binary % 1000000;
    return QString("%1").arg(password, 6, 10, QChar('0'));
}


Entry::Entry()
    : m_attributes(new EntryAttributes(this))
    , m_attachments(new EntryAttachments(this))
    , m_autoTypeAssociations(new AutoTypeAssociations(this))
    , m_tmpHistoryItem(nullptr)
    , m_modifiedSinceBegin(false)
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
        m_data.timeInfo.setLastModificationTime(QDateTime::currentDateTimeUtc());
        m_data.timeInfo.setLastAccessTime(QDateTime::currentDateTimeUtc());
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
        Q_ASSERT(database());

        if (database()) {
            return database()->metadata()->customIcon(m_data.customIcon);
        }
        else {
            return QImage();
        }
    }
}

QPixmap Entry::iconPixmap() const
{
    if (m_data.customIcon.isNull()) {
        return databaseIcons()->iconPixmap(m_data.iconNumber);
    }
    else {
        Q_ASSERT(database());

        if (database()) {
            return database()->metadata()->customIconPixmap(m_data.customIcon);
        }
        else {
            return QPixmap();
        }
    }
}

QPixmap Entry::iconScaledPixmap() const
{
    if (m_data.customIcon.isNull()) {
        // built-in icons are 16x16 so don't need to be scaled
        return databaseIcons()->iconPixmap(m_data.iconNumber);
    }
    else {
        Q_ASSERT(database());

        return database()->metadata()->customIconScaledPixmap(m_data.customIcon);
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
    return m_attributes->value(EntryAttributes::TitleKey);
}

QString Entry::url() const
{
    return m_attributes->value(EntryAttributes::URLKey);
}

QString Entry::username() const
{
    return m_attributes->value(EntryAttributes::UserNameKey);
}

QString Entry::password() const
{
    return m_attributes->value(EntryAttributes::PasswordKey);
}

QString Entry::notes() const
{
    return m_attributes->value(EntryAttributes::NotesKey);
}

bool Entry::isExpired() const
{
    return m_data.timeInfo.expires() && m_data.timeInfo.expiryTime() < QDateTime::currentDateTimeUtc();
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

bool Entry::hasTOTP() {
    return m_attributes->hasKey("TOTP Seed");
}

QString Entry::getTOTP(){
    if (hasTOTP()) {
        return OTPgeneratePin(m_attributes->value("TOTP Seed").toLatin1());
    }
}

void Entry::setSeedTOTP(QString seed){
    m_attributes->set("TOTP Seed", seed);
    m_attributes->set("TOTP Settings","30;6");
}

QString Entry::seed() {
    return m_attributes->value("TOTP Seed");
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
    m_attributes->set(EntryAttributes::TitleKey, title, m_attributes->isProtected(EntryAttributes::TitleKey));
}

void Entry::setUrl(const QString& url)
{
    m_attributes->set(EntryAttributes::URLKey, url, m_attributes->isProtected(EntryAttributes::URLKey));
}

void Entry::setUsername(const QString& username)
{
    m_attributes->set(EntryAttributes::UserNameKey, username, m_attributes->isProtected(EntryAttributes::UserNameKey));
}

void Entry::setPassword(const QString& password)
{
    m_attributes->set(EntryAttributes::PasswordKey, password, m_attributes->isProtected(EntryAttributes::PasswordKey));
}

void Entry::setNotes(const QString& notes)
{
    m_attributes->set(EntryAttributes::NotesKey, notes, m_attributes->isProtected(EntryAttributes::NotesKey));
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

    m_history.append(entry);
    Q_EMIT modified();
}

void Entry::removeHistoryItems(const QList<Entry*>& historyEntries)
{
    if (historyEntries.isEmpty()) {
        return;
    }

    Q_FOREACH (Entry* entry, historyEntries) {
        Q_ASSERT(!entry->parent());
        Q_ASSERT(entry->uuid() == uuid());
        Q_ASSERT(m_history.contains(entry));

        m_history.removeOne(entry);
        delete entry;
    }

    Q_EMIT modified();
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

Entry* Entry::clone(CloneFlags flags) const
{
    Entry* entry = new Entry();
    entry->setUpdateTimeinfo(false);
    if (flags & CloneNewUuid) {
        entry->m_uuid = Uuid::random();
    }
    else {
        entry->m_uuid = m_uuid;
    }
    entry->m_data = m_data;
    entry->m_attributes->copyDataFrom(m_attributes);
    entry->m_attachments->copyDataFrom(m_attachments);
    entry->m_autoTypeAssociations->copyDataFrom(this->m_autoTypeAssociations);
    if (flags & CloneIncludeHistory) {
        Q_FOREACH (Entry* historyItem, m_history) {
            Entry* historyItemClone = historyItem->clone(flags & ~CloneIncludeHistory & ~CloneNewUuid);
            historyItemClone->setUpdateTimeinfo(false);
            historyItemClone->setUuid(entry->uuid());
            historyItemClone->setUpdateTimeinfo(true);
            entry->addHistoryItem(historyItemClone);
        }
    }
    entry->setUpdateTimeinfo(true);

    if (flags & CloneResetTimeInfo) {
        QDateTime now = QDateTime::currentDateTimeUtc();
        entry->m_data.timeInfo.setCreationTime(now);
        entry->m_data.timeInfo.setLastModificationTime(now);
        entry->m_data.timeInfo.setLastAccessTime(now);
        entry->m_data.timeInfo.setLocationChanged(now);
    }



    return entry;
}

void Entry::copyDataFrom(const Entry* other)
{
    setUpdateTimeinfo(false);
    m_data = other->m_data;
    m_attributes->copyDataFrom(other->m_attributes);
    m_attachments->copyDataFrom(other->m_attachments);
    m_autoTypeAssociations->copyDataFrom(other->m_autoTypeAssociations);
    setUpdateTimeinfo(true);
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

bool Entry::endUpdate()
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

    m_tmpHistoryItem = nullptr;

    return m_modifiedSinceBegin;
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
        m_data.timeInfo.setLocationChanged(QDateTime::currentDateTimeUtc());
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
        return nullptr;
    }
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
