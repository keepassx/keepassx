/*
 *  Copyright (C) 2016 Lennart Glauer <mail@lennart-glauer.de>
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

#ifndef KEEPASSX_APPKIT_INTERFACE_H
#define KEEPASSX_APPKIT_INTERFACE_H

#include <unistd.h>

extern "C" {

class AppKitWrapper
{
public:
    AppKitWrapper();
    ~AppKitWrapper();

    pid_t lastActiveProcessId();
    pid_t ownProcessId();
    pid_t activeProcessId();
    bool activateProcess(pid_t pid);
    bool isProcessActive(pid_t pid);

private:
    void *self;
};

} // extern "C"

#endif // KEEPASSX_APPKIT_INTERFACE_H