/*
 *  Copyright (C) 2016 Enrico Mariotti <enricomariotti@yahoo.it>
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

#include "CsvParserModel.h"

CsvParserModel::CsvParserModel(QObject *parent)
    : QAbstractTableModel(parent)
    , m_skipped(0)
{}

CsvParserModel::~CsvParserModel()
{}

void CsvParserModel::setFilename(const QString& filename) {
    m_filename = filename;
}

QString CsvParserModel::getFileInfo(){
    QString a(QString::number(getFileSize()).append(tr(" byte, ")));
    a.append(QString::number(getCsvRows())).append(tr(" rows, "));
    a.append(QString::number((getCsvCols()-1))).append(tr(" columns"));
    return a;
}

bool CsvParserModel::parse() {
    bool r;
    beginResetModel();
    m_columnMap.clear();
    if (CsvParser::isFileLoaded()) {
        r = CsvParser::reparse();
    }
    else {
        QFile csv(m_filename);
        r = CsvParser::parse(&csv);
    }
    for (int i=0; i<columnCount(); i++) {
        m_columnMap.insert(i,0);
    }
    addEmptyColumn();
    endResetModel();
    return r;
}

void CsvParserModel::addEmptyColumn() {
    for (int i=0; i<m_table.size(); ++i) {
        csvrow r = m_table.at(i);
        r.prepend(QString(""));
        m_table.replace(i, r);
    }
}

void CsvParserModel::mapColumns(int csvColumn, int dbColumn) {
    if ((csvColumn < 0) || (dbColumn < 0) ) {
        return;
    }

    beginResetModel();
    if (csvColumn >= getCsvCols()) {
        m_columnMap[dbColumn] = 0; //map to the empty column
    }
    else {
        m_columnMap[dbColumn] = csvColumn;
    }
    endResetModel();
}

void CsvParserModel::setSkippedRows(int skipped) {
    m_skipped = skipped;
    QModelIndex topLeft = createIndex(skipped,0);
    QModelIndex bottomRight = createIndex(m_skipped+rowCount(), columnCount());
    Q_EMIT dataChanged(topLeft, bottomRight);
    Q_EMIT layoutChanged();
}

void CsvParserModel::setHeaderLabels(QStringList l) {
    m_columnHeader = l;
}

int CsvParserModel::rowCount(const QModelIndex &parent) const {
    if (parent.isValid()) {
        return 0;
    }
    return getCsvRows();
}

int CsvParserModel::columnCount(const QModelIndex &parent) const {
    if (parent.isValid()) {
        return 0;
    }
    return m_columnHeader.size();
}

QVariant CsvParserModel::data(const QModelIndex &index, int role) const {
    if (   (index.column() >= m_columnHeader.size())
        || (index.row()+m_skipped >= rowCount())
        || !index.isValid() )
    {
        return QVariant();
    }

    if (role == Qt::DisplayRole) {
        return m_table.at(index.row()+m_skipped).at(m_columnMap[index.column()]);
    }
    return QVariant();
}

QVariant CsvParserModel::headerData(int section, Qt::Orientation orientation, int role) const {
    if (role == Qt::DisplayRole) {
        if (orientation == Qt::Horizontal) {
            if ( (section < 0) || (section >= m_columnHeader.size())) {
                return QVariant();
            }
            return m_columnHeader.at(section);
        }
        else if (orientation == Qt::Vertical) {
            if (section+m_skipped >= rowCount()) {
                return QVariant();
            }
            return QString::number(section+1);
        }
    }
    return QVariant();
}


