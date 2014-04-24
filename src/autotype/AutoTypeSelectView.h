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

#ifndef KEEPASSX_AUTOTYPESELECTVIEW_H
#define KEEPASSX_AUTOTYPESELECTVIEW_H

#include "core/Global.h"
#include "gui/entry/EntryView.h"

class Entry;

class AutoTypeSelectView : public EntryView
{
    Q_OBJECT

public:
    explicit AutoTypeSelectView(QWidget* parent = Q_NULLPTR);
    virtual const QString& getHeaderConfigKeyName();
    
    static const QString m_HEADER_CONFIG_KEY_NAME;

protected:
    void mouseMoveEvent(QMouseEvent* event) Q_DECL_OVERRIDE;

private Q_SLOTS:
    void selectFirstEntry();
};

#endif // KEEPASSX_AUTOTYPESELECTVIEW_H
