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

#ifndef KEEPASSX_SHORTCUTWIDGET_H
#define KEEPASSX_SHORTCUTWIDGET_H

#include <QtCore/QtGlobal>

#if QT_VERSION >= QT_VERSION_CHECK(5, 0, 0)
#include <QtWidgets/QLineEdit>
#else
#include <QtGui/QLineEdit>
#endif

#include "core/Global.h"

class ShortcutWidget : public QLineEdit
{
    Q_OBJECT

public:
    explicit ShortcutWidget(QWidget* parent = Q_NULLPTR);
    Qt::Key key() const;
    Qt::KeyboardModifiers modifiers() const;
    void setShortcut(Qt::Key key, Qt::KeyboardModifiers modifiers);

protected:
    void keyPressEvent(QKeyEvent* event) Q_DECL_OVERRIDE;
    void keyReleaseEvent(QKeyEvent* event) Q_DECL_OVERRIDE;

private:
    void keyEvent(QKeyEvent* event);
    void displayShortcut(Qt::Key key, Qt::KeyboardModifiers modifiers);
    void resetShortcut();

    Qt::Key m_key;
    Qt::KeyboardModifiers m_modifiers;
    bool m_locked;
};

#endif // KEEPASSX_SHORTCUTWIDGET_H
