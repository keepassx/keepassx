/*
 *  Copyright (C) 2013 Tom Tijerina <tijerina.tom@gmail.com>
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

#include "PasswordPreviewDialog.h"
#include "ui_PasswordPreviewDialog.h"

PasswordPreviewDialog::PasswordPreviewDialog(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::PasswordPreviewDialog)
{
    ui->setupUi(this);

    connect(ui->buttonClose, SIGNAL(clicked()), SLOT(close()));
}

PasswordPreviewDialog::~PasswordPreviewDialog()
{
    delete ui;
}

void PasswordPreviewDialog::setMaximumProgressBar(int maximumProgress){
    ui->progressBar->setMaximum(maximumProgress);
}

void PasswordPreviewDialog::setProgressBar(int progress){
    ui->progressBar->setValue(progress);
}

void PasswordPreviewDialog::appendPasswordList(QString newPassword){
    ui->passwordPreviews->appendPlainText(newPassword);
}


