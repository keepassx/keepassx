include_directories(SYSTEM ${X11_X11_INCLUDE_PATH})

set(qt5_libs)
if (WITH_QT5)
    find_package(Qt5X11Extras REQUIRED)
    list(APPEND qt5_libs Qt5X11Extras)
    include_directories(SYSTEM ${Qt5X11Extras_INCLUDE_DIRS})
endif ()

set(autotype_X11_SOURCES
    AutoTypeX11.cpp
)

set(autotype_X11_MOC
    AutoTypeX11.h
)

qt_wrap_cpp(autotype_X11_SOURCES ${autotype_X11_MOC})

add_library(keepassx-autotype-x11 MODULE ${autotype_X11_SOURCES})
target_link_libraries(keepassx-autotype-x11 ${QT_QTCORE_LIBRARY} ${QT_QTGUI_LIBRARY} ${QT_QTWIDGETS_LIBRARY} ${qt5_libs} ${X11_X11_LIB} ${X11_XTest_LIB})
install(TARGETS keepassx-autotype-x11
        BUNDLE DESTINATION . COMPONENT Runtime
        LIBRARY DESTINATION ${PLUGIN_INSTALL_DIR} COMPONENT Runtime)
