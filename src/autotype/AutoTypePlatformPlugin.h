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

#ifndef KEEPASSX_AUTOTYPEPLATFORMPLUGIN_H
#define KEEPASSX_AUTOTYPEPLATFORMPLUGIN_H

#include <QWidget>

#include "autotype/AutoTypeAction.h"

class AutoTypePlatformInterface
{
public:
    virtual ~AutoTypePlatformInterface() {}
    virtual QStringList windowTitles() = 0;
    virtual WId activeWindow() = 0;
    virtual QString activeWindowTitle() = 0;
    virtual bool registerGlobalShortcut(Qt::Key key, Qt::KeyboardModifiers modifiers) = 0;
    virtual void unregisterGlobalShortcut(Qt::Key key, Qt::KeyboardModifiers modifiers) = 0;
    virtual int platformEventFilter(void* event) = 0;
    virtual int initialTimeout() = 0;
    virtual void unload() {}

    virtual AutoTypeExecutor* createExecutor() = 0;

    // implementations should also provide a globalShortcutTriggered() signal
};

Q_DECLARE_INTERFACE(AutoTypePlatformInterface, "org.keepassx.AutoTypePlatformInterface/1")

#endif // KEEPASSX_AUTOTYPEPLATFORMPLUGIN_H
