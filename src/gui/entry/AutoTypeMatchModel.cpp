/*
 *  Copyright (C) 2015 David Wu <lightvector@gmail.com>
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

#include "AutoTypeMatchModel.h"

#include <QFont>

#include "core/DatabaseIcons.h"
#include "core/Entry.h"
#include "core/Group.h"
#include "core/Metadata.h"

AutoTypeMatchModel::AutoTypeMatchModel(QObject* parent)
    : QAbstractTableModel(parent)
{
}

AutoTypeMatch AutoTypeMatchModel::matchFromIndex(const QModelIndex& index) const
{
    Q_ASSERT(index.isValid() && index.row() < m_matches.size());
    return m_matches.at(index.row());
}

QModelIndex AutoTypeMatchModel::indexFromMatch(AutoTypeMatch match) const
{
    int row = m_matches.indexOf(match);
    Q_ASSERT(row != -1);
    return index(row, 1);
}

void AutoTypeMatchModel::setMatchList(const QList<AutoTypeMatch>& matches)
{
    beginResetModel();

    severConnections();

    m_allGroups.clear();
    m_matches = matches;

    QSet<Database*> databases;

    Q_FOREACH (AutoTypeMatch match, m_matches) {
        databases.insert(match.entry->group()->database());
    }

    Q_FOREACH (Database* db, databases) {
        Q_ASSERT(db);
        Q_FOREACH (const Group* group, db->rootGroup()->groupsRecursive(true)) {
            m_allGroups.append(group);
        }

        if (db->metadata()->recycleBin()) {
            m_allGroups.removeOne(db->metadata()->recycleBin());
        }
    }

    Q_FOREACH (const Group* group, m_allGroups) {
        makeConnections(group);
    }

    endResetModel();
}

int AutoTypeMatchModel::rowCount(const QModelIndex& parent) const
{
    if (parent.isValid()) {
        return 0;
    }
    else {
        return m_matches.size();
    }
}

int AutoTypeMatchModel::columnCount(const QModelIndex& parent) const
{
    Q_UNUSED(parent);

    return 4;
}

QVariant AutoTypeMatchModel::data(const QModelIndex& index, int role) const
{
    if (!index.isValid()) {
        return QVariant();
    }

    AutoTypeMatch match = matchFromIndex(index);

    if (role == Qt::DisplayRole) {
        switch (index.column()) {
        case ParentGroup:
            if (match.entry->group()) {
                return match.entry->group()->name();
            }
            break;
        case Title:
            return match.entry->title();
        case Username:
            return match.entry->username();
        case Sequence:
            return match.sequence;
        }
    }
    else if (role == Qt::DecorationRole) {
        switch (index.column()) {
        case ParentGroup:
            if (match.entry->group()) {
                return match.entry->group()->iconPixmap();
            }
            break;
        case Title:
            if (match.entry->isExpired()) {
                return databaseIcons()->iconPixmap(DatabaseIcons::ExpiredIconIndex);
            }
            else {
                return match.entry->iconPixmap();
            }
        }
    }
    else if (role == Qt::FontRole) {
        QFont font;
        if (match.entry->isExpired()) {
            font.setStrikeOut(true);
        }
        return font;
    }

    return QVariant();
}

QVariant AutoTypeMatchModel::headerData(int section, Qt::Orientation orientation, int role) const
{
    if (orientation == Qt::Horizontal && role == Qt::DisplayRole) {
        switch (section) {
        case ParentGroup:
            return tr("Group");
        case Title:
            return tr("Title");
        case Username:
            return tr("Username");
        case Sequence:
            return tr("Sequence");
        }
    }

    return QVariant();
}

void AutoTypeMatchModel::entryDataChanged(Entry* entry)
{
    for(int row = 0; row < m_matches.size(); row++) {
        AutoTypeMatch match = m_matches[row];
        if(match.entry == entry) {
            Q_EMIT dataChanged(index(row, 0), index(row, columnCount()-1));
        }
    }
}


void AutoTypeMatchModel::entryAboutToRemove(Entry* entry)
{
    for(int row = 0; row < m_matches.size(); row++) {
        AutoTypeMatch match = m_matches[row];
        if(match.entry == entry) {
            beginRemoveRows(QModelIndex(), row, row);
            m_matches.removeAt(row);
            endRemoveRows();
            row--;
        }
    }
}

void AutoTypeMatchModel::entryRemoved()
{
    //empty
}

void AutoTypeMatchModel::severConnections()
{
    Q_FOREACH (const Group* group, m_allGroups) {
        disconnect(group, nullptr, this, nullptr);
    }
}

void AutoTypeMatchModel::makeConnections(const Group* group)
{
    connect(group, SIGNAL(entryAboutToRemove(Entry*)), SLOT(entryAboutToRemove(Entry*)));
    connect(group, SIGNAL(entryRemoved(Entry*)), SLOT(entryRemoved()));
    connect(group, SIGNAL(entryDataChanged(Entry*)), SLOT(entryDataChanged(Entry*)));
}
