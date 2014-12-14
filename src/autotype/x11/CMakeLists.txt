include_directories(SYSTEM ${X11_X11_INCLUDE_PATH})

set(autotype_X11_SOURCES
    AutoTypeX11.cpp
)

set(autotype_X11_MOC
    AutoTypeX11.h
)

if(USE_QT5)
  qt5_wrap_cpp(autotype_X11_SOURCES ${autotype_X11_MOC})
else()
  qt4_wrap_cpp(autotype_X11_SOURCES ${autotype_X11_MOC})
endif()

add_library(keepassx-autotype-x11 MODULE ${autotype_X11_SOURCES})
if(USE_QT5)
  target_link_libraries(keepassx-autotype-x11 Qt5::Widgets Qt5::X11Extras ${X11_X11_LIB} ${X11_XTest_LIB})
else()
  target_link_libraries(keepassx-autotype-x11 ${QT_QTCORE_LIBRARY} ${QT_QTGUI_LIBRARY} ${X11_X11_LIB} ${X11_XTest_LIB})
endif()
install(TARGETS keepassx-autotype-x11
        BUNDLE DESTINATION . COMPONENT Runtime
        LIBRARY DESTINATION ${PLUGIN_INSTALL_DIR} COMPONENT Runtime)
