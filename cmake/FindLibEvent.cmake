#  Copyright (C) 2016 Yong-Siang Shih <shaform@gmail.com>
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

find_path(EVENT_INCLUDE_DIR event2/event.h
  PATHS /usr/include
)

find_library(EVENT_LIBRARY
  NAMES event
  PATHS /usr/lib /usr/local/lib
)

mark_as_advanced(EVENT_INCLUDE_DIR EVENT_LIBRARY)

include(FindPackageHandleStandardArgs)
find_package_handle_standard_args(EVENT DEFAULT_MSG EVENT_INCLUDE_DIR EVENT_LIBRARY)
