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

#ifndef KEEPASSX_EDITWIDGETPROPERTIES_H
#define KEEPASSX_EDITWIDGETPROPERTIES_H

#include <QWidget>

#include "core/TimeInfo.h"
#include "core/Uuid.h"

namespace Ui {
    class EditWidgetProperties;
}

class EditWidgetProperties : public QWidget
{
    Q_OBJECT

public:
    explicit EditWidgetProperties(QWidget* parent = nullptr);
    ~EditWidgetProperties();

    void setFields(TimeInfo timeInfo, Uuid uuid);

private:
    const QScopedPointer<Ui::EditWidgetProperties> m_ui;

    Q_DISABLE_COPY(EditWidgetProperties)
};

#endif // KEEPASSX_EDITWIDGETPROPERTIES_H
