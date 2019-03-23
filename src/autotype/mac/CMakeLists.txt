set(autotype_mac_SOURCES
    AutoTypeMac.cpp
)

set(autotype_mac_mm_SOURCES
    AppKitImpl.mm
)

add_library(keepassx-autotype-cocoa MODULE ${autotype_mac_SOURCES} ${autotype_mac_mm_SOURCES})
set_target_properties(keepassx-autotype-cocoa PROPERTIES LINK_FLAGS "-framework Foundation -framework AppKit -framework Carbon")
target_link_libraries(keepassx-autotype-cocoa ${PROGNAME} Qt5::Core Qt5::Widgets)
install(TARGETS keepassx-autotype-cocoa
        BUNDLE DESTINATION . COMPONENT Runtime
        LIBRARY DESTINATION ${PLUGIN_INSTALL_DIR} COMPONENT Runtime)
install(CODE "
    include(BundleUtilities)
    fixup_bundle(\"\${CMAKE_INSTALL_PREFIX}/${PROGNAME}.app\" \"\${CMAKE_INSTALL_PREFIX}/${PLUGIN_INSTALL_DIR}/libkeepassx-autotype-cocoa.so\" \"\")
" COMPONENT Runtime)