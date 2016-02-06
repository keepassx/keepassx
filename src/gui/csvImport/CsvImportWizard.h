/*
 *  Copyright (C) 2015 Enrico Mariotti <enricomariotti@yahoo.it>
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

#ifndef KEEPASSX_CSVIMPORTWIZARD_H
#define KEEPASSX_CSVIMPORTWIZARD_H

#include <QDebug>

#include <QStackedWidget>
#include <QGridLayout>

#include "CsvImportWidget.h"
#include "core/Database.h"
#include "gui/ChangeMasterKeyWidget.h"
#include "gui/DialogyWidget.h"

class CsvImportWidget;

class CsvImportWizard : public DialogyWidget
{
    Q_OBJECT

public:
    explicit CsvImportWizard(QWidget *parent = nullptr);
    virtual ~CsvImportWizard();
    void load(const QString& filename, Database *database);

Q_SIGNALS:
    void importFinished(bool accepted);

private Q_SLOTS:
    void keyFinished(bool accepted);
    void parseFinished(bool accepted);

private:
    Database* m_db;
    CsvImportWidget* parse;
    ChangeMasterKeyWidget* key;
    QStackedWidget *m_pages;
    QGridLayout *m_layout;
};

#endif //KEEPASSX_CSVIMPORTWIZARD_H
