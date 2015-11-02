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

#ifndef KEEPASSX_SETTINGSWIDGET_H
#define KEEPASSX_SETTINGSWIDGET_H

#include "gui/EditWidget.h"

namespace Ui {
    class SettingsWidgetGeneral;
    class SettingsWidgetSecurity;
}

class SettingsWidget : public EditWidget
{
    Q_OBJECT

public:
    explicit SettingsWidget(QWidget* parent = nullptr);
    ~SettingsWidget();
    void loadSettings();

Q_SIGNALS:
    void editFinished(bool accepted);

private Q_SLOTS:
    void saveSettings();
    void reject();
    void enableAutoSaveOnExit(bool checked);

private:
    QWidget* const m_secWidget;
    QWidget* const m_generalWidget;
    const QScopedPointer<Ui::SettingsWidgetSecurity> m_secUi;
    const QScopedPointer<Ui::SettingsWidgetGeneral> m_generalUi;
    Qt::Key m_globalAutoTypeKey;
    Qt::KeyboardModifiers m_globalAutoTypeModifiers;
};

#endif // KEEPASSX_SETTINGSWIDGET_H
