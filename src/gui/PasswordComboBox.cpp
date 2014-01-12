/*
 *  Copyright (C) 2013 
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

#include <QDebug>
#include <QLineEdit>

#include "core/PasswordGenerator.h"

PasswordComboBox::PasswordComboBox(QWidget* parent ) : QComboBox( parent )
  , m_generator( 0 )
  , m_alternatives( 10 )
{
  setEditable( true );
  setEcho( false );

  connect(this, SIGNAL(activated(int)), SLOT(changeText(int)));
}

PasswordComboBox::~PasswordComboBox()
{
}

void PasswordComboBox::addObfuscated( QString password )
{
  QString display = password;
  if( !m_echo ) {
    display = QString( password.length(), L'·' );
  }

  addItem( display, password );
}

void PasswordComboBox::changeText( int index )
{
  setEditText( itemData( index ).toString() );
}

void PasswordComboBox::clear()
{
  QString current = currentText();

  QComboBox::clear();

  addObfuscated( current );
}

void PasswordComboBox::setEcho( bool echo )
{
  m_echo = echo;

  lineEdit()->setEchoMode( echo ? QLineEdit::Normal : QLineEdit::Password );
}

void PasswordComboBox::setGenerator( PasswordGenerator *generator )
{
  m_generator = generator;
}

void PasswordComboBox::setNumberAlternatives( int alternatives )
{
  m_alternatives = alternatives;
}

void PasswordComboBox::showPopup()
{
    clear();

    if( m_generator && m_generator->isValid() ) {

      for( int alternative = 0; alternative < m_alternatives; ++alternative ) {
        QString password = m_generator->generatePassword();

        addObfuscated( password );
      }
    }

    QComboBox::showPopup();
}
