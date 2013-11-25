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

#ifndef PREVIEWGUI_H
#define PREVIEWGUI_H

#include <QDialog>

#include "core/Global.h"
#include "core/PasswordGenerator.h"

namespace Ui {
class PasswordPreviewDialog;
}

class PasswordPreviewDialog : public QDialog
{
    Q_OBJECT

public:
    void setMaximumProgressBar(int maximumProgress);
    void setProgressBar(int progress);
    void appendPasswordList(QString newPassword);
    explicit PasswordPreviewDialog(QWidget *parent);
    ~PasswordPreviewDialog();

private:
    Ui::PasswordPreviewDialog *ui;
};

#endif // PREVIEWGUI_H
