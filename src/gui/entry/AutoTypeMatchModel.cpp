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

#include "AutoTypeMatchModel.h"

#include <QFont>

#include "core/DatabaseIcons.h"
#include "core/Entry.h"
#include "core/Group.h"

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
    m_matches = matches;
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


