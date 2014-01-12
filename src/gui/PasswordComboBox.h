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

#ifndef KEEPASSX_PASSWORDCOMBOBOX_H
#define KEEPASSX_PASSWORDCOMBOBOX_H

#include <QComboBox>

#include "core/Global.h"

class PasswordGenerator;

class PasswordComboBox : public QComboBox
{
    Q_OBJECT
public:
    explicit PasswordComboBox(QWidget* parent = Q_NULLPTR);
    virtual ~PasswordComboBox();

    virtual void clear();

    void setEcho( bool echo );

    void setGenerator( PasswordGenerator *generator );

    void setNumberAlternatives( int alternatives );

    virtual void showPopup();

private Q_SLOTS:
  void changeText( int );

private:
  void addObfuscated( QString password );

  PasswordGenerator *m_generator;

  int m_alternatives;

  bool m_echo;
};

#endif
