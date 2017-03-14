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

#include <format/KeePass2.h>
#include <crypto/kdf/Kdf.h>
#include <crypto/kdf/Argon2Kdf.h>
#include "DatabaseSettingsWidget.h"
#include "ui_DatabaseSettingsWidget.h"

#include "core/Database.h"
#include "core/Group.h"
#include "core/Metadata.h"
#include "keys/CompositeKey.h"
#include "MessageBox.h"

DatabaseSettingsWidget::DatabaseSettingsWidget(QWidget* parent)
    : DialogyWidget(parent)
    , m_ui(new Ui::DatabaseSettingsWidget())
    , m_db(nullptr)
{
    m_ui->setupUi(this);

    connect(m_ui->buttonBox, SIGNAL(accepted()), SLOT(save()));
    connect(m_ui->buttonBox, SIGNAL(rejected()), SLOT(reject()));
    connect(m_ui->historyMaxItemsCheckBox, SIGNAL(toggled(bool)),
            m_ui->historyMaxItemsSpinBox, SLOT(setEnabled(bool)));
    connect(m_ui->historyMaxSizeCheckBox, SIGNAL(toggled(bool)),
            m_ui->historyMaxSizeSpinBox, SLOT(setEnabled(bool)));
    connect(m_ui->transformBenchmarkButton, SIGNAL(clicked()), SLOT(transformRoundsBenchmark()));
    connect(m_ui->kdfCombo, SIGNAL(currentIndexChanged(int)), SLOT(changeKdf(int)));
}

DatabaseSettingsWidget::~DatabaseSettingsWidget()
{
}

void DatabaseSettingsWidget::load(Database* db)
{
    m_db = db;

    Metadata* meta = m_db->metadata();

    m_ui->dbNameEdit->setText(meta->name());
    m_ui->dbDescriptionEdit->setText(meta->description());
    m_ui->recycleBinEnabledCheckBox->setChecked(meta->recycleBinEnabled());
    m_ui->defaultUsernameEdit->setText(meta->defaultUserName());
    if (meta->historyMaxItems() > -1) {
        m_ui->historyMaxItemsSpinBox->setValue(meta->historyMaxItems());
        m_ui->historyMaxItemsCheckBox->setChecked(true);
    }
    else {
        m_ui->historyMaxItemsSpinBox->setValue(Metadata::DefaultHistoryMaxItems);
        m_ui->historyMaxItemsCheckBox->setChecked(false);
    }
    int historyMaxSizeMiB = qRound(meta->historyMaxSize() / qreal(1048576));
    if (historyMaxSizeMiB > 0) {
        m_ui->historyMaxSizeSpinBox->setValue(historyMaxSizeMiB);
        m_ui->historyMaxSizeCheckBox->setChecked(true);
    }
    else {
        m_ui->historyMaxSizeSpinBox->setValue(Metadata::DefaultHistoryMaxSize);
        m_ui->historyMaxSizeCheckBox->setChecked(false);
    }

    // FIXME don't hardcode this here
    m_ui->cipherCombo->addItem("AES-256", KeePass2::CIPHER_AES.toByteArray());
    m_ui->cipherCombo->addItem("ChaCha20", KeePass2::CIPHER_CHACHA20.toByteArray());
    int cipherIndex = m_ui->cipherCombo->findData(m_db->cipher().toByteArray());
    if (cipherIndex > -1) {
        m_ui->cipherCombo->setCurrentIndex(cipherIndex);
    }

    m_ui->kdfCombo->addItem("AES-KDF", QVariant(KeePass2::KDF_AES.toByteArray()));
    m_ui->kdfCombo->addItem("Argon2", QVariant(KeePass2::KDF_ARGON2.toByteArray()));
    QVariantMap& kdfParams = m_db->kdfParams();
    Uuid curKdf = Uuid(kdfParams.value(KeePass2::KDFPARAM_UUID).toByteArray());
    int kdfIndex = m_ui->kdfCombo->findData(curKdf.toByteArray()); // FIXME format details here :(
    if (kdfIndex > -1) {
        m_ui->kdfCombo->setCurrentIndex(kdfIndex);
    }
    if (curKdf == KeePass2::KDF_AES) {
        m_ui->transformRoundsSpinBox->setValue(kdfParams.value(KeePass2::KDFPARAM_AES_ROUNDS).toInt());
    } else if (curKdf == KeePass2::KDF_ARGON2) {
        m_ui->transformRoundsSpinBox->setValue(kdfParams.value(KeePass2::KDFPARAM_ARGON2_TIME).toInt());
        m_ui->memoryCostSpinBox->setValue(kdfParams.value(KeePass2::KDFPARAM_ARGON2_MEMORY).toInt());
        m_ui->lanesSpinBox->setValue(kdfParams.value(KeePass2::KDFPARAM_ARGON2_LANES).toInt());
    }

    m_ui->dbNameEdit->setFocus();
}

void DatabaseSettingsWidget::save()
{
    Metadata* meta = m_db->metadata();

    meta->setName(m_ui->dbNameEdit->text());
    meta->setDescription(m_ui->dbDescriptionEdit->text());
    meta->setDefaultUserName(m_ui->defaultUsernameEdit->text());
    meta->setRecycleBinEnabled(m_ui->recycleBinEnabledCheckBox->isChecked());
    meta->setSettingsChanged(QDateTime::currentDateTimeUtc());

    bool truncate = false;

    int historyMaxItems;
    if (m_ui->historyMaxItemsCheckBox->isChecked()) {
        historyMaxItems = m_ui->historyMaxItemsSpinBox->value();
    }
    else {
        historyMaxItems = -1;
    }
    if (historyMaxItems != meta->historyMaxItems()) {
        meta->setHistoryMaxItems(historyMaxItems);
        truncate = true;
    }

    int historyMaxSize;
    if (m_ui->historyMaxSizeCheckBox->isChecked()) {
        historyMaxSize = m_ui->historyMaxSizeSpinBox->value() * 1048576;
    }
    else {
        historyMaxSize = -1;
    }
    if (historyMaxSize != meta->historyMaxSize()) {
        meta->setHistoryMaxSize(historyMaxSize);
        truncate = true;
    }

    if (truncate) {
        truncateHistories();
    }

    m_db->setCipher(Uuid(m_ui->cipherCombo->currentData().toByteArray()));
    // FIXME we shouldn't be dealing with the raw map here although KeePass2 itself also does it..
    QVariantMap kdfParams(m_db->kdfParams());
    Uuid selKdf(m_ui->kdfCombo->currentData().toByteArray());
    if (kdfParams.value(KeePass2::KDFPARAM_UUID).toByteArray() != selKdf.toByteArray()) {
        QScopedPointer<Kdf> kdf(Kdf::getKdf(selKdf));
        kdfParams = kdf->defaultParams();
        kdf->randomizeSalt(kdfParams);
    }

    if (selKdf == KeePass2::KDF_AES) {
        kdfParams.insert(KeePass2::KDFPARAM_AES_ROUNDS, static_cast<quint64>(m_ui->transformRoundsSpinBox->value()));
    } else if (selKdf == KeePass2::KDF_ARGON2) {
        kdfParams.insert(KeePass2::KDFPARAM_ARGON2_TIME, static_cast<quint64>(m_ui->transformRoundsSpinBox->value()));
        kdfParams.insert(KeePass2::KDFPARAM_ARGON2_MEMORY, static_cast<quint64>(m_ui->memoryCostSpinBox->value()));
        kdfParams.insert(KeePass2::KDFPARAM_ARGON2_LANES, static_cast<quint32>(m_ui->lanesSpinBox->value()));
    }
    QApplication::setOverrideCursor(QCursor(Qt::WaitCursor));
    bool newParamsOk = m_db->changeKdfParams(kdfParams);
    QApplication::restoreOverrideCursor();
    if (!newParamsOk) {
        MessageBox::warning(this, tr("Error"), tr("Invalid key derivation function parameters;\nparameters left unchanged."));
    }

    Q_EMIT editFinished(true);
}

void DatabaseSettingsWidget::reject()
{
    Q_EMIT editFinished(false);
}

void DatabaseSettingsWidget::transformRoundsBenchmark()
{
    QApplication::setOverrideCursor(QCursor(Qt::WaitCursor));
    int rounds = -1;
    Uuid selKdf(m_ui->kdfCombo->currentData().toByteArray());
    if (selKdf == KeePass2::KDF_AES) {
        rounds = CompositeKey::transformKeyAesBenchmark(1000);
    } else if (selKdf == KeePass2::KDF_ARGON2) {
        rounds = CompositeKey::transformKeyArgon2Benchmark(1000, m_ui->lanesSpinBox->value(), m_ui->memoryCostSpinBox->value());
    }

    if (rounds != -1) {
        m_ui->transformRoundsSpinBox->setValue(rounds);
    }
    QApplication::restoreOverrideCursor();
}

void DatabaseSettingsWidget::truncateHistories()
{
    const QList<Entry*> allEntries = m_db->rootGroup()->entriesRecursive(false);
    for (Entry* entry : allEntries) {
        entry->truncateHistory();
    }
}

void DatabaseSettingsWidget::changeKdf(int index) {
    // FIXME format details here again
    Uuid selKdf(m_ui->kdfCombo->itemData(index).toByteArray());
    bool vis = selKdf == KeePass2::KDF_ARGON2;
    m_ui->memoryCostLabel->setVisible(vis);
    m_ui->memoryCostSpinBox->setVisible(vis);
    m_ui->lanesLabel->setVisible(vis);
    m_ui->lanesSpinBox->setVisible(vis);
}
