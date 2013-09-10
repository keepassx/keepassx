/*
 *  Copyright (C) 2010 Felix Geyer <debfx@fobos.de>
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

#ifndef KEEPASSX_MAINWINDOW_H
#define KEEPASSX_MAINWINDOW_H

#include <QtGui/QActionGroup>
#include <QtGui/QMainWindow>

#include <QtGui/QSystemTrayIcon>
#include <QtGui/QMenu>

#include "core/SignalMultiplexer.h"
#include "gui/DatabaseWidget.h"

namespace Ui {
    class MainWindow;
}

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow();
    ~MainWindow();

public Q_SLOTS:
    void openDatabase(const QString& fileName, const QString& pw = QString(),
                      const QString& keyFile = QString());

protected:
     void closeEvent(QCloseEvent* event) Q_DECL_OVERRIDE;

private Q_SLOTS:
    void setMenuActionState(DatabaseWidget::Mode mode = DatabaseWidget::None);
    void updateWindowTitle();
    void showAboutDialog();
    void switchToDatabases();
    void switchToSettings();
    void databaseTabChanged(int tabIndex);
    void openRecentDatabase(QAction* action);
    void clearLastDatabases();
    void updateLastDatabasesMenu();
    void updateCopyAttributesMenu();
    void showEntryContextMenu(const QPoint& globalPos);
    void showGroupContextMenu(const QPoint& globalPos);
    void saveToolbarState(bool value);
    void rememberOpenDatabases(const QString& filePath);
    void createTrayIcon();
    void setIcon();
    void trayActivated(QSystemTrayIcon::ActivationReason reason);


private:
    static void setShortcut(QAction* action, QKeySequence::StandardKey standard, int fallback = 0);

    static const QString BaseWindowTitle;

    const QScopedPointer<Ui::MainWindow> m_ui;
    SignalMultiplexer m_actionMultiplexer;
    QAction* m_clearHistoryAction;
    QActionGroup* m_lastDatabasesActions;
    QActionGroup* m_copyAdditionalAttributeActions;
    QStringList m_openDatabases;
    
    QAction* quitAction;
    
    QSystemTrayIcon* trayIcon;
    QMenu* trayIconMenu;

    Q_DISABLE_COPY(MainWindow)
};

#endif // KEEPASSX_MAINWINDOW_H
