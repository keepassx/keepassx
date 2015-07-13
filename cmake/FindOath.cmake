#  Copyright (C) 2015 Felix Geyer <debfx@fobos.de>
#
#  This program is free software: you can redistribute it and/or modify
#  it under the terms of the GNU General Public License as published by
#  the Free Software Foundation, either version 2 or (at your option)
#  version 3 of the License.
#
#  This program is distributed in the hope that it will be useful,
#  but WITHOUT ANY WARRANTY; without even the implied warranty of
#  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
#  GNU General Public License for more details.
#
#  You should have received a copy of the GNU General Public License
#  along with this program.  If not, see <http://www.gnu.org/licenses/>.

find_path(OATH_INCLUDE_DIR liboath/oath.h)

find_library(OATH_LIBRARIES oath)

mark_as_advanced(OATH_LIBRARIES OATH_INCLUDE_DIR)

include(FindPackageHandleStandardArgs)
find_package_handle_standard_args(Oath DEFAULT_MSG OATH_LIBRARIES OATH_INCLUDE_DIR)
