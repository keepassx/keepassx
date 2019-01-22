set(autotype_mac_SOURCES
    AutoTypeMac.cpp
)

set(autotype_mac_MOC
    AutoTypeMac.h
)

qt4_wrap_cpp(autotype_mac_SOURCES ${autotype_mac_MOC})

add_library(keepassx-autotype-mac MODULE ${autotype_mac_SOURCES})
set_target_properties(keepassx-autotype-mac PROPERTIES LINK_FLAGS "-framework Foundation -framework AppKit -framework Carbon")
target_link_libraries(keepassx-autotype-mac keepassx_core ${QT_QTCORE_LIBRARY} ${QT_QTGUI_LIBRARY})
