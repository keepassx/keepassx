set(autotype_mac_SOURCES
    AutoTypeMac.cpp
)

set(autotype_mac_mm_SOURCES
    AppKitWrapper.mm
)

add_library(keepassx-autotype-cocoa MODULE ${autotype_mac_SOURCES} ${autotype_mac_mm_SOURCES})
set_target_properties(keepassx-autotype-cocoa PROPERTIES LINK_FLAGS "-framework Foundation -framework AppKit -framework Carbon")
target_link_libraries(keepassx-autotype-cocoa ${PROGNAME} Qt5::Core Qt5::Widgets)
install(TARGETS keepassx-autotype-cocoa
        BUNDLE DESTINATION . COMPONENT Runtime
        LIBRARY DESTINATION ${PLUGIN_INSTALL_DIR} COMPONENT Runtime)