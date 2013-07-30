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

#include "DialogyWidget.h"

#if QT_VERSION >= QT_VERSION_CHECK(5, 0, 0)
#include <QtWidgets/QDialogButtonBox>
#include <QtWidgets/QPushButton>
#else
#include <QtGui/QDialogButtonBox>
#include <QtGui/QPushButton>
#endif
#include <QtGui/QKeyEvent>

DialogyWidget::DialogyWidget(QWidget* parent)
    : QWidget(parent)
{
}

void DialogyWidget::keyPressEvent(QKeyEvent* e)
{
#ifdef Q_OS_MAC
    if (e->modifiers() == Qt::ControlModifier && e->key() == Qt::Key_Period) {
        if (!clickButton(QDialogButtonBox::Cancel)) {
            e->ignore();
        }
    }
    else
#endif
    if (!e->modifiers() || (e->modifiers() & Qt::KeypadModifier && e->key() == Qt::Key_Enter)) {
        switch (e->key()) {
            case Qt::Key_Enter:
            case Qt::Key_Return:
                if (!clickButton(QDialogButtonBox::Ok)) {
                    e->ignore();
                }
                break;
            case Qt::Key_Escape:
                if (!clickButton(QDialogButtonBox::Cancel)) {
                    e->ignore();
                }
                break;
            default:
                e->ignore();
        }
    }
    else {
        e->ignore();
    }
}

bool DialogyWidget::clickButton(QDialogButtonBox::StandardButton standardButton)
{
    QPushButton* pb = qobject_cast<QPushButton*>(focusWidget());
    if (pb && pb->isVisible() && pb->isEnabled() && pb->hasFocus()) {
        pb->click();
        return true;
    }

    QList<QDialogButtonBox*> buttonBoxes = findChildren<QDialogButtonBox*>();
    for (int i = 0; i < buttonBoxes.size(); ++i) {
        QDialogButtonBox* buttonBox = buttonBoxes.at(i);
        pb = buttonBox->button(standardButton);
        if (pb && pb->isVisible() && pb->isEnabled()) {
            pb->click();
            return true;
        }
    }

    return false;
}
