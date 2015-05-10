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

#ifndef KEEPASSX_SORTFILTERHIDEPROXYMODEL_H
#define KEEPASSX_SORTFILTERHIDEPROXYMODEL_H

#include <QBitArray>
#include <QSortFilterProxyModel>

#include "core/Global.h"

class SortFilterHideProxyModel : public QSortFilterProxyModel
{
    Q_OBJECT

public:
    explicit SortFilterHideProxyModel(QObject* parent = Q_NULLPTR);
    void hideColumn(int column, bool hide);
#if (QT_VERSION >= QT_VERSION_CHECK(5, 0, 0))
    Qt::DropActions supportedDragActions() const Q_DECL_OVERRIDE;
#endif

protected:
    bool filterAcceptsColumn(int sourceColumn, const QModelIndex& sourceParent) const Q_DECL_OVERRIDE;

private:
    QBitArray m_hiddenColumns;
};

#endif // KEEPASSX_SORTFILTERHIDEPROXYMODEL_H
