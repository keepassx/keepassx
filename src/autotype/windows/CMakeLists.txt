set(autotype_win_SOURCES
    AutoTypeWindows.cpp
)

add_library(keepassx-autotype-windows MODULE ${autotype_win_SOURCES})

# KEEPASSX_BUILDING_CORE -> KEEPASSX_EXPORT Q_DECL_EXPORT (see core/Global.h)
set_target_properties(keepassx-autotype-windows PROPERTIES COMPILE_DEFINITIONS KEEPASSX_BUILDING_CORE)

target_link_libraries(keepassx-autotype-windows keepassx_core Qt5::Core Qt5::Widgets)
install(TARGETS keepassx-autotype-windows
        BUNDLE DESTINATION . COMPONENT Runtime
        LIBRARY DESTINATION ${PLUGIN_INSTALL_DIR} COMPONENT Runtime)