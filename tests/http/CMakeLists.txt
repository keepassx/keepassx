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

set(httptestsupport_SOURCES HttpUtil.cpp)
add_library(httptestsupport STATIC ${httptestsupport_SOURCES})
target_link_libraries(httptestsupport Qt5::Core Qt5::Concurrent Qt5::Test)

add_unit_test(NAME testhttpserver SOURCES TestHttpServer.cpp LIBS httptestsupport ${TEST_LIBRARIES})
add_unit_test(NAME testrequest SOURCES TestRequest.cpp LIBS ${TEST_LIBRARIES})
add_unit_test(NAME testresponse SOURCES TestResponse.cpp LIBS ${TEST_LIBRARIES})
add_unit_test(NAME testserver SOURCES TestServer.cpp LIBS httptestsupport ${TEST_LIBRARIES})
if(WITH_GUI_TESTS)
  add_unit_test(NAME testservice SOURCES TestService.cpp LIBS httptestsupport ${TEST_LIBRARIES})
endif(WITH_GUI_TESTS)
