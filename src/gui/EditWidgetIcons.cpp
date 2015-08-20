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

#include "EditWidgetIcons.h"
#include "ui_EditWidgetIcons.h"

#include <QFileDialog>
#include <QMessageBox>
#include <QBuffer>
#include <QXmlStreamReader>

#include "core/Group.h"
#include "core/Metadata.h"
#include "core/Tools.h"
#include "gui/IconModels.h"
#include "gui/MessageBox.h"

IconStruct::IconStruct()
    : uuid(Uuid())
    , number(0)
{
}

EditWidgetIcons::EditWidgetIcons(QWidget* parent)
    : QWidget(parent)
    , m_ui(new Ui::EditWidgetIcons())
    , m_database(Q_NULLPTR)
    , m_defaultIconModel(new DefaultIconModel(this))
    , m_customIconModel(new CustomIconModel(this))
    , m_networkAccessMngr(new QNetworkAccessManager(this))
{
    m_ui->setupUi(this);

    m_ui->defaultIconsView->setModel(m_defaultIconModel);
    m_ui->customIconsView->setModel(m_customIconModel);

    connect(m_ui->defaultIconsView, SIGNAL(clicked(QModelIndex)),
            this, SLOT(updateRadioButtonDefaultIcons()));
    connect(m_ui->customIconsView, SIGNAL(clicked(QModelIndex)),
            this, SLOT(updateRadioButtonCustomIcons()));
    connect(m_ui->defaultIconsRadio, SIGNAL(toggled(bool)),
            this, SLOT(updateWidgetsDefaultIcons(bool)));
    connect(m_ui->customIconsRadio, SIGNAL(toggled(bool)),
            this, SLOT(updateWidgetsCustomIcons(bool)));
    connect(m_ui->addButton, SIGNAL(clicked()), SLOT(addCustomIcon()));
    connect(m_ui->deleteButton, SIGNAL(clicked()), SLOT(removeCustomIcon()));
    connect(m_ui->faviconButton, SIGNAL(clicked()), SLOT(downloadFavicon()));
    connect(m_networkAccessMngr, SIGNAL(finished(QNetworkReply*)),
            this, SLOT(onRequestFinished(QNetworkReply*)) );

    m_ui->faviconButton->setVisible(false);
}

EditWidgetIcons::~EditWidgetIcons()
{
}

IconStruct EditWidgetIcons::save()
{
    Q_ASSERT(m_database);
    Q_ASSERT(!m_currentUuid.isNull());

    IconStruct iconStruct;
    if (m_ui->defaultIconsRadio->isChecked()) {
        QModelIndex index = m_ui->defaultIconsView->currentIndex();
        if (index.isValid()) {
            iconStruct.number = index.row();
        }
        else {
            Q_ASSERT(false);
        }
    }
    else {
        QModelIndex index = m_ui->customIconsView->currentIndex();
        if (index.isValid()) {
            iconStruct.uuid = m_customIconModel->uuidFromIndex(m_ui->customIconsView->currentIndex());
        }
        else {
            iconStruct.number = -1;
        }
    }

    m_database = Q_NULLPTR;
    m_currentUuid = Uuid();
    return iconStruct;
}

void EditWidgetIcons::load(Uuid currentUuid, Database* database, IconStruct iconStruct, const QString &url)
{
    Q_ASSERT(database);
    Q_ASSERT(!currentUuid.isNull());

    m_database = database;
    m_currentUuid = currentUuid;
    setUrl(url);

    m_customIconModel->setIcons(database->metadata()->customIcons(),
                                database->metadata()->customIconsOrder());

    Uuid iconUuid = iconStruct.uuid;
    if (iconUuid.isNull()) {
        int iconNumber = iconStruct.number;
        m_ui->defaultIconsView->setCurrentIndex(m_defaultIconModel->index(iconNumber, 0));
        m_ui->defaultIconsRadio->setChecked(true);
    }
    else {
        QModelIndex index = m_customIconModel->indexFromUuid(iconUuid);
        if (index.isValid()) {
            m_ui->customIconsView->setCurrentIndex(index);
            m_ui->customIconsRadio->setChecked(true);
        }
        else {
            m_ui->defaultIconsView->setCurrentIndex(m_defaultIconModel->index(0, 0));
            m_ui->defaultIconsRadio->setChecked(true);
        }
    }
}

void EditWidgetIcons::setUrl(const QString &url)
{
    m_url = QUrl(url);
    // We do not want to send any user information to google
    m_ui->faviconButton->setVisible(m_url.userInfo().isEmpty() && (m_url.scheme() == "http" || m_url.scheme() == "https"));
    abortFaviconDownload();
}

void EditWidgetIcons::downloadFavicon()
{
    // We do not want to send any user information to google
    Q_ASSERT(m_url.userInfo().isEmpty());

    const QStringList schemes = QStringList() << "http" << "https";
    const QStringList extensions = QStringList() << "ico" << "png" << "gif" << "jpg";

    // This might be requested again in the loop below, but is is not an issue
    m_networkOperations << m_networkAccessMngr->get(QNetworkRequest(m_url));

    QString domain = m_url.host();
    int firstDot;
    int lastDot;
    do {
        m_networkOperations << m_networkAccessMngr->get(QNetworkRequest("http://www.google.com/s2/favicons?domain=" + domain));
        Q_FOREACH (QString scheme, schemes) {
            m_networkOperations << m_networkAccessMngr->get(QNetworkRequest(scheme + "://" + domain + "/"));
            Q_FOREACH (QString extension, extensions) {
                m_networkOperations << m_networkAccessMngr->get(QNetworkRequest(scheme + "://" + domain + "/favicon." + extension));
                if (m_url.port() >= 0)
                    m_networkOperations << m_networkAccessMngr->get(QNetworkRequest(scheme + "://" + domain + ":" + m_url.port() + "/favicon." + extension));
            }
        }
        firstDot = domain.indexOf( '.' );
        lastDot = domain.lastIndexOf( '.' );
        domain.remove( 0, firstDot + 1 );
    } while (( firstDot != -1 ) && ( lastDot != -1 ) && ( firstDot != lastDot ));
    m_progress = new QProgressDialog(this);
    connect(m_progress, SIGNAL(canceled()), this, SLOT(abortFaviconDownload()));
    m_progress->setWindowModality(Qt::WindowModal);
    m_progress->setAutoClose(true);
    m_progress->setLabelText("Downloading Favicons...");
    m_progress->setMaximum(m_networkOperations.count());
    m_progress->setValue(m_progress->maximum() - m_networkOperations.count());
    m_progress->show();
}

void EditWidgetIcons::abortFaviconDownload()
{
    Q_FOREACH (QNetworkReply *r, m_networkOperations)
        r->abort();
}

void EditWidgetIcons::onRequestFinished(QNetworkReply *reply)
{
    Q_ASSERT(m_networkOperations.contains(reply));

    const QByteArray data = reply->readAll();
    const QUrl originalUrl = reply->url();
    m_networkOperations.remove(reply);
    reply->deleteLater();
     m_progress->setValue(m_progress->maximum() - m_networkOperations.count());
    if (reply->error()) return;

    QImage image;
    if (image.loadFromData(data) && !image.isNull()) {
        // Checking for Duplicates
        // Convert to 16x16 PNG, so that we can compare with existing ones
        QByteArray ba1;
        QBuffer buffer1(&ba1);
        buffer1.open(QIODevice::WriteOnly);
        image.scaled(16, 16).save(&buffer1, "PNG");
        buffer1.close();
        image.loadFromData(ba1);

        QHash<Uuid, QImage>::const_iterator i;
        for (i = m_database->metadata()->customIcons().constBegin(); i != m_database->metadata()->customIcons().constEnd(); ++i) {
            if (i.value() == image) return;
            QByteArray ba2;
            QBuffer buffer2(&ba2);
            buffer2.open(QIODevice::WriteOnly);
            i.value().save(&buffer1, "PNG");
            buffer2.close();
            if (ba1 == ba2) return;
        }
        Uuid uuid = Uuid::random();
        m_database->metadata()->addCustomIcon(uuid, image);
        m_customIconModel->setIcons(m_database->metadata()->customIcons(),
                                    m_database->metadata()->customIconsOrder());
        QModelIndex index = m_customIconModel->indexFromUuid(uuid);
        m_ui->customIconsView->setCurrentIndex(index);
        m_ui->customIconsRadio->setChecked(true);
    } else { // It might be a WebPage
        // XML approach does not even work on https://github.com/
        // I got "Expected '=', but got '>'." while parsing the <head>
        /*
        QXmlStreamReader page(data);
        if(page.error() || !page.readNextStartElement() || page.name() != "html" || !page.readNextStartElement() || page.name() != "head")
            return;
        while (!page.atEnd()) {
            page.readNextStartElement();
            if (page.name() == "link") {
                const QXmlStreamAttributes attrs = page.attributes();
                const QString rel = attrs.value("rel").toString();
                if (rel == "icon" || rel == "shortcut icon" || rel == "apple-touch-icon") {
                    m_networkOperations << m_networkAccessMngr->get(QNetworkRequest(attrs.value("href").toString()));
                }
            }

        }
        */
        QString page(data);
        int pos = page.indexOf("<head");
        const int headEnd = qMin(page.indexOf("</head>"), page.indexOf("<body"));
        // It does not work with rel and href in reverse order
        QRegExp link("<link[^>]+rel=\"(icon|shortcut icon|apple-touch-icon)\"[^>]+href=\"([^\"]+)\"");
        while ( (pos = link.indexIn(page, pos)) != -1 ) {
            if (pos >= headEnd) return;
            pos += link.matchedLength();
            QUrl iconUrl(link.cap(2));
            if (iconUrl.isRelative()) {
                m_networkOperations << m_networkAccessMngr->get(QNetworkRequest(originalUrl.resolved(iconUrl)));
            } else {
                m_networkOperations << m_networkAccessMngr->get(QNetworkRequest(iconUrl));
            }
        }
    }
}

void EditWidgetIcons::addCustomIcon()
{
    if (m_database) {
        QString filter = QString("%1 (%2);;%3 (*)").arg(tr("Images"),
                    Tools::imageReaderFilter(), tr("All files"));

        QString filename = QFileDialog::getOpenFileName(
                    this, tr("Select Image"), "", filter);
        if (!filename.isEmpty()) {
            QImage image(filename);
            if (!image.isNull()) {
                Uuid uuid = Uuid::random();
                m_database->metadata()->addCustomIconScaled(uuid, image);
                m_customIconModel->setIcons(m_database->metadata()->customIcons(),
                                            m_database->metadata()->customIconsOrder());
                QModelIndex index = m_customIconModel->indexFromUuid(uuid);
                m_ui->customIconsView->setCurrentIndex(index);
            }
            else {
                // TODO: show error
            }
        }
    }
}

void EditWidgetIcons::removeCustomIcon()
{
    if (m_database) {
        QModelIndex index = m_ui->customIconsView->currentIndex();
        if (index.isValid()) {
            Uuid iconUuid = m_customIconModel->uuidFromIndex(index);
            int iconUsedCount = 0;

            QList<Entry*> allEntries = m_database->rootGroup()->entriesRecursive(true);
            QList<Entry*> historyEntriesWithSameIcon;

            Q_FOREACH (Entry* entry, allEntries) {
                bool isHistoryEntry = !entry->group();
                if (iconUuid == entry->iconUuid()) {
                    if (isHistoryEntry) {
                        historyEntriesWithSameIcon << entry;
                    }
                    else if (m_currentUuid != entry->uuid()) {
                        iconUsedCount++;
                    }
                }
            }

            QList<Group*> allGroups = m_database->rootGroup()->groupsRecursive(true);
            Q_FOREACH (const Group* group, allGroups) {
                if (iconUuid == group->iconUuid() && m_currentUuid != group->uuid()) {
                    iconUsedCount++;
                }
            }

            if (iconUsedCount == 0) {
                Q_FOREACH (Entry* entry, historyEntriesWithSameIcon) {
                    entry->setUpdateTimeinfo(false);
                    entry->setIcon(0);
                    entry->setUpdateTimeinfo(true);
                }

                m_database->metadata()->removeCustomIcon(iconUuid);
                m_customIconModel->setIcons(m_database->metadata()->customIcons(),
                                            m_database->metadata()->customIconsOrder());
                if (m_customIconModel->rowCount() > 0) {
                    m_ui->customIconsView->setCurrentIndex(m_customIconModel->index(0, 0));
                }
                else {
                    updateRadioButtonDefaultIcons();
                }
            }
            else {
                MessageBox::information(this, tr("Can't delete icon!"),
                                        tr("Can't delete icon. Still used by %n item(s).", 0, iconUsedCount));
            }
        }
    }
}

void EditWidgetIcons::updateWidgetsDefaultIcons(bool check)
{
    if (check) {
        QModelIndex index = m_ui->defaultIconsView->currentIndex();
        if (!index.isValid()) {
            m_ui->defaultIconsView->setCurrentIndex(m_defaultIconModel->index(0, 0));
        }
        else {
            m_ui->defaultIconsView->setCurrentIndex(index);
        }
        m_ui->customIconsView->selectionModel()->clearSelection();
        m_ui->addButton->setEnabled(false);
        m_ui->deleteButton->setEnabled(false);
    }
}

void EditWidgetIcons::updateWidgetsCustomIcons(bool check)
{
    if (check) {
        QModelIndex index = m_ui->customIconsView->currentIndex();
        if (!index.isValid()) {
            m_ui->customIconsView->setCurrentIndex(m_customIconModel->index(0, 0));
        }
        else {
            m_ui->customIconsView->setCurrentIndex(index);
        }
        m_ui->defaultIconsView->selectionModel()->clearSelection();
        m_ui->addButton->setEnabled(true);
        m_ui->deleteButton->setEnabled(true);
    }
}

void EditWidgetIcons::updateRadioButtonDefaultIcons()
{
    m_ui->defaultIconsRadio->setChecked(true);
}

void EditWidgetIcons::updateRadioButtonCustomIcons()
{
    m_ui->customIconsRadio->setChecked(true);
}
