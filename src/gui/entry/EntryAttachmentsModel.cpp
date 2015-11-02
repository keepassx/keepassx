/*
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

#include "EntryAttachmentsModel.h"

#include "core/Entry.h"
#include "core/Tools.h"

EntryAttachmentsModel::EntryAttachmentsModel(QObject* parent)
    : QAbstractListModel(parent)
    , m_entryAttachments(nullptr)
{
}

void EntryAttachmentsModel::setEntryAttachments(EntryAttachments* entryAttachments)
{
    beginResetModel();

    if (m_entryAttachments) {
        m_entryAttachments->disconnect(this);
    }

    m_entryAttachments = entryAttachments;

    if (m_entryAttachments) {
        connect(m_entryAttachments, SIGNAL(keyModified(QString)), SLOT(attachmentChange(QString)));
        connect(m_entryAttachments, SIGNAL(aboutToBeAdded(QString)), SLOT(attachmentAboutToAdd(QString)));
        connect(m_entryAttachments, SIGNAL(added(QString)), SLOT(attachmentAdd()));
        connect(m_entryAttachments, SIGNAL(aboutToBeRemoved(QString)), SLOT(attachmentAboutToRemove(QString)));
        connect(m_entryAttachments, SIGNAL(removed(QString)), SLOT(attachmentRemove()));
        connect(m_entryAttachments, SIGNAL(aboutToBeReset()), SLOT(aboutToReset()));
        connect(m_entryAttachments, SIGNAL(reset()), SLOT(reset()));
    }

    endResetModel();
}

int EntryAttachmentsModel::rowCount(const QModelIndex& parent) const
{
    if (!m_entryAttachments || parent.isValid()) {
        return 0;
    }
    else {
        return m_entryAttachments->keys().size();
    }
}

int EntryAttachmentsModel::columnCount(const QModelIndex& parent) const
{
    Q_UNUSED(parent);

    return 1;
}

QVariant EntryAttachmentsModel::data(const QModelIndex& index, int role) const
{
    if (!index.isValid()) {
        return QVariant();
    }

    if (role == Qt::DisplayRole && index.column() == 0) {
        QString key = keyByIndex(index);

        return QString("%1 (%2)").arg(key,
                Tools::humanReadableFileSize(m_entryAttachments->value(key).size()));
    }
    else {
        return QVariant();
    }
}

QString EntryAttachmentsModel::keyByIndex(const QModelIndex& index) const
{
    if (!index.isValid()) {
        return QString();
    }

    return m_entryAttachments->keys().at(index.row());
}

void EntryAttachmentsModel::attachmentChange(const QString& key)
{
    int row = m_entryAttachments->keys().indexOf(key);
    Q_EMIT dataChanged(index(row, 0), index(row, columnCount()-1));
}

void EntryAttachmentsModel::attachmentAboutToAdd(const QString& key)
{
    QList<QString> rows = m_entryAttachments->keys();
    rows.append(key);
    qSort(rows);
    int row = rows.indexOf(key);
    beginInsertRows(QModelIndex(), row, row);
}

void EntryAttachmentsModel::attachmentAdd()
{
    endInsertRows();
}

void EntryAttachmentsModel::attachmentAboutToRemove(const QString& key)
{
    int row = m_entryAttachments->keys().indexOf(key);
    beginRemoveRows(QModelIndex(), row, row);
}

void EntryAttachmentsModel::attachmentRemove()
{
    endRemoveRows();
}

void EntryAttachmentsModel::aboutToReset()
{
    beginResetModel();
}

void EntryAttachmentsModel::reset()
{
    endResetModel();
}
