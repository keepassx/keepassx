/*
 *  Copyright (C) 2013 Michael Curtis <michael@moltenmercury.org>
 *  Copyright (C) 2014 Felix Geyer <debfx@fobos.de>
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

#include "PasswordComboBox.h"

#include <QLineEdit>

#include "core/PassphraseGenerator.h"
#include "core/PasswordGenerator.h"

PasswordComboBox::PasswordComboBox(QWidget* parent)
    : QComboBox(parent)
    , m_password_generator(nullptr)
    , m_passphrase_generator(nullptr)
    , m_alternatives(10)
{
    setEditable(true);
    setEcho(false);
}

PasswordComboBox::~PasswordComboBox()
{
}

void PasswordComboBox::setEcho(bool echo)
{
    lineEdit()->setEchoMode(echo ? QLineEdit::Normal : QLineEdit::Password);

    QString current = currentText();

    if (echo) {
        // add fake item to show visual indication that a popup is available
        addItem("");

#ifdef Q_OS_MAC
        // Qt on Mac OS doesn't seem to know the generic monospace family (tested with 4.8.6)
        setStyleSheet("QComboBox { font-family: monospace,Menlo,Monaco; }");
#else
        setStyleSheet("QComboBox { font-family: monospace,Courier New; }");
#endif
    }
    else {
        // clear items so the combobox indicates that no popup menu is available
        clear();

        setStyleSheet("QComboBox { font-family: initial; }");
    }

    setEditText(current);
}

void PasswordComboBox::setGenerator(PasswordGenerator* generator)
{
    m_password_generator = generator;
    m_passphrase_generator = nullptr;
}

void PasswordComboBox::setGenerator(PassphraseGenerator* generator)
{
  m_password_generator = nullptr;
  m_passphrase_generator = generator;
}

void PasswordComboBox::setNumberAlternatives(int alternatives)
{
    m_alternatives = alternatives;
}

void PasswordComboBox::showPopup()
{
    // no point in showing a bunch of hidden passwords
    if (lineEdit()->echoMode() == QLineEdit::Password) {
        hidePopup();
        return;
    }

    // keep existing password as the first item in the popup
    QString current = currentText();
    clear();
    addItem(current);

    if (m_password_generator && m_password_generator->isValid()) {
        for (int alternative = 0; alternative < m_alternatives; alternative++) {
            QString password = m_password_generator->generatePassword();
            addItem(password);
        }
    } else if (m_passphrase_generator && m_passphrase_generator->isValid()) {
      for (int alternative = 0; alternative < m_alternatives; alternative++) {
          QString passphrase = m_passphrase_generator->generatePassphrase();
          addItem(passphrase);
      }
    }

    QComboBox::showPopup();
}
