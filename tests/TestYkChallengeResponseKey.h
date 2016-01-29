/*
 *  Copyright (C) 2014 Kyle Manna <kyle@kylemanna.com>
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

#ifndef KEEPASSX_TESTYUBIKEYCHALRESP_H
#define KEEPASSX_TESTYUBIKEYCHALRESP_H

#include <QObject>

#include "keys/YkChallengeResponseKey.h"

class TestYubiKeyChalResp: public QObject
{
    Q_OBJECT

private Q_SLOTS:
    void initTestCase();
    void cleanupTestCase();

    void init();

    /* Order is important!
     * Need to init and detectDevices() before proceeding
     */
    void detectDevices();

    void getSerial();
    void keyGetName();
    void keyIssueChallenge();

    void deinit();

    /* Callback for detectDevices() */
    void ykDetected(int slot, bool blocking);

private:
    int m_detected;
    YkChallengeResponseKey *m_key;
};

#endif // KEEPASSX_TESTYUBIKEYCHALRESP_H
