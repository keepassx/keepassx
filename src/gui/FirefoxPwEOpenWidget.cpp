/*
 *  Copyright (C) 2014 Karsten Hinz <k.hinz@tu-bs.de>
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

#include "FirefoxPwEOpenWidget.h"

#include <QFile>
#include <QFileInfo>

#include "ui_DatabaseOpenWidget.h"
#include "core/Database.h"
#include "core/Metadata.h"
#include "core/Entry.h"
#include "core/Group.h"
#include "format/FirefoxPwEReader.h"
#include "gui/MessageBox.h"

FirefoxPwEOpenWidget::FirefoxPwEOpenWidget(QWidget* parent)
    : DatabaseOpenWidget(parent)
{
    m_ui->labelHeadline->setText(tr("Import Password file exported by Firefox Addon 'Password Exporter'"));
    m_ui->buttonBox->button(QDialogButtonBox::Ok)->setEnabled(true);
    m_ui->comboKeyFile->setEnabled(false);
    m_ui->checkKeyFile->setEnabled(false);
    m_ui->checkPassword->setEnabled(false);
    m_ui->buttonBrowseFile->setEnabled(false);
    m_ui->editPassword->setEnabled(false);
}


void FirefoxPwEOpenWidget::openDatabase()
{

    FirefoxPwEReader reader;

    QString password;

    if (m_ui->checkPassword->isChecked()) {
        password = m_ui->editPassword->text();
    }

    QFile file(m_filename);
    if (!file.open(QIODevice::ReadOnly)) {
        // TODO: error message
        return;
    }
    if (m_db) {
        delete m_db;
    }
    QApplication::setOverrideCursor(QCursor(Qt::WaitCursor));

    // get database
    Database* db = reader.readDatabase(&file, password);

    if (!reader.hasError()) {
        m_db = db;
    }

    //continue
    QApplication::restoreOverrideCursor();

    if (m_db) {
        m_db->metadata()->setName(QFileInfo(m_filename).completeBaseName());
        Q_EMIT editFinished(true);
    }
    else {
        MessageBox::warning(this, tr("Error"), tr("Unable to open the database.").append("\n")
                .append(reader.errorString()));
        m_ui->editPassword->clear();
    }
}
