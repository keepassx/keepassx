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

#ifndef KEEPASSX_DIALOGYWIDGET_H
#define KEEPASSX_DIALOGYWIDGET_H

#include <QtCore/QtGlobal>

#if QT_VERSION >= QT_VERSION_CHECK(5, 0, 0)
#include <QtWidgets/QDialogButtonBox>
#include <QtWidgets/QWidget>
#else
#include <QtGui/QDialogButtonBox>
#include <QtGui/QWidget>
#endif

#include "core/Global.h"

class DialogyWidget : public QWidget
{
    Q_OBJECT

public:
    explicit DialogyWidget(QWidget* parent = Q_NULLPTR);

protected:
    virtual void keyPressEvent(QKeyEvent* e) Q_DECL_OVERRIDE;

private:
    bool clickButton(QDialogButtonBox::StandardButton standardButton);
};

#endif // KEEPASSX_DIALOGYWIDGET_H
