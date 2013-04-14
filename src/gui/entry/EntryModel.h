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

#ifndef KEEPASSX_ENTRYMODEL_H
#define KEEPASSX_ENTRYMODEL_H

#include <QtCore/QAbstractTableModel>

#include "core/Global.h"

class Entry;
class Group;

class EntryModel : public QAbstractTableModel
{
    Q_OBJECT

public:
    enum ModelColumn
    {
        ParentGroup = 0,
        Title = 1,
        Username = 2,
        Url = 3
    };

    explicit EntryModel(QObject* parent = Q_NULLPTR);
    Entry* entryFromIndex(const QModelIndex& index) const;
    QModelIndex indexFromEntry(Entry* entry) const;

    int rowCount(const QModelIndex& parent = QModelIndex()) const Q_DECL_OVERRIDE;
    int columnCount(const QModelIndex& parent = QModelIndex()) const Q_DECL_OVERRIDE;
    QVariant data(const QModelIndex& index, int role = Qt::DisplayRole) const Q_DECL_OVERRIDE;
    QVariant headerData(int section, Qt::Orientation orientation, int role = Qt::DisplayRole) const Q_DECL_OVERRIDE;
    Qt::DropActions supportedDropActions() const Q_DECL_OVERRIDE;
    Qt::ItemFlags flags(const QModelIndex& modelIndex) const Q_DECL_OVERRIDE;
    QStringList mimeTypes() const Q_DECL_OVERRIDE;
    QMimeData* mimeData(const QModelIndexList& indexes) const Q_DECL_OVERRIDE;

    void setEntryList(const QList<Entry*>& entries);

Q_SIGNALS:
    void switchedToEntryListMode();
    void switchedToGroupMode();

public Q_SLOTS:
    void setGroup(Group* group);

private Q_SLOTS:
    void entryAboutToAdd(Entry* entry);
    void entryAdded(Entry* entry);
    void entryAboutToRemove(Entry* entry);
    void entryRemoved();
    void entryDataChanged(Entry* entry);

private:
    void severConnections();
    void makeConnections(const Group* group);

    Group* m_group;
    QList<Entry*> m_entries;
    QList<Entry*> m_orgEntries;
    QList<const Group*> m_allGroups;
};

#endif // KEEPASSX_ENTRYMODEL_H
