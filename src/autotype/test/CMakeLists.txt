set(autotype_test_SOURCES
    AutoTypeTest.cpp
)

set(autotype_test_MOC
    AutoTypeTest.h
)

if(USE_QT5)
  qt5_wrap_cpp(autotype_test_SOURCES ${autotype_test_MOC})
else()
  qt4_wrap_cpp(autotype_test_SOURCES ${autotype_test_MOC})
endif()

add_library(keepassx-autotype-test MODULE ${autotype_test_SOURCES})
if(USE_QT5)
  target_link_libraries(keepassx-autotype-test testautotype Qt5::Widgets)
else()
  target_link_libraries(keepassx-autotype-test testautotype ${QT_QTCORE_LIBRARY} ${QT_QTGUI_LIBRARY})
endif()
