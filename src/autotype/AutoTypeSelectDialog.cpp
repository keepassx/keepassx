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

#include "AutoTypeSelectDialog.h"

#include <QApplication>
#include <QDesktopWidget>
#include <QDialogButtonBox>
#include <QLabel>
#include <QVBoxLayout>

#include "autotype/AutoTypeSelectView.h"
#include "core/AutoTypeMatch.h"
#include "core/FilePath.h"
#include "gui/entry/AutoTypeMatchModel.h"

AutoTypeSelectDialog::AutoTypeSelectDialog(QWidget* parent)
    : QDialog(parent)
    , m_view(new AutoTypeSelectView(this))
    , m_matchActivatedEmitted(false)
{
    setAttribute(Qt::WA_DeleteOnClose);
    setWindowFlags(windowFlags() | Qt::WindowStaysOnTopHint);
    setWindowTitle(tr("Auto-Type - KeePassX"));
    setWindowIcon(filePath()->applicationIcon());

    QSize size(600, 250);
    resize(size);

    // move dialog to the center of the screen
    QPoint screenCenter = QApplication::desktop()->screenGeometry(QCursor::pos()).center();
    move(screenCenter.x() - (size.width() / 2), screenCenter.y() - (size.height() / 2));

    QVBoxLayout* layout = new QVBoxLayout(this);

    QLabel* descriptionLabel = new QLabel(tr("Select entry to Auto-Type:"), this);
    layout->addWidget(descriptionLabel);

    connect(m_view, SIGNAL(activated(QModelIndex)), SLOT(emitMatchActivated(QModelIndex)));
    connect(m_view, SIGNAL(clicked(QModelIndex)), SLOT(emitMatchActivated(QModelIndex)));
    layout->addWidget(m_view);

    QDialogButtonBox* buttonBox = new QDialogButtonBox(QDialogButtonBox::Cancel, Qt::Horizontal, this);
    connect(buttonBox, SIGNAL(rejected()), SLOT(reject()));
    layout->addWidget(buttonBox);
}

void AutoTypeSelectDialog::setMatchList(const QList<AutoTypeMatch>& matchList)
{
    m_view->setMatchList(matchList);
}

void AutoTypeSelectDialog::emitMatchActivated(const QModelIndex& index)
{
    // make sure we don't emit the signal twice when both activated() and clicked() are triggered
    if (m_matchActivatedEmitted) {
        return;
    }
    m_matchActivatedEmitted = true;

    AutoTypeMatch match = m_view->matchFromIndex(index);
    accept();
    Q_EMIT matchActivated(match);
}
