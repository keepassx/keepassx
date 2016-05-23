#!/bin/bash

# Install xcode
# Install homebrew
# > brew install cmake qt5 libgcrypt

if cd build; then
    export DYLD_PRINT_LIBRARIES=1
    export CMAKE_PREFIX_PATH=/usr/local/opt/qt5/
    cmake .. -DCMAKE_INSTALL_PREFIX=/usr/local -DCMAKE_BUILD_TYPE=Release
    make -j4

    QT5_DIR="/usr/local/opt/qt5"
    BREW_QT5_DIR="/usr/local/Cellar/qt5/5.6.0"
    LIBGCRYPT_DIR="/usr/local/opt/libgcrypt"
    APP_DIR="src/KeePassX.app"
    PLUGINS_DIR="$APP_DIR/Contents/PlugIns"
    PLATFORMS_DIR="$PLUGINS_DIR/platforms"

    mkdir $PLUGINS_DIR
    mkdir $PLATFORMS_DIR
    cp $QT5_DIR/plugins/platforms/libqcocoa.dylib $APP_DIR/Contents/PlugIns/platforms/libqcocoa.dylib

    # fix id
    install_name_tool -id @executable_path/../PlugIns/platforms/libqcocoa.dylib \
        $PLATFORMS_DIR/libqcocoa.dylib

    # fix dependencies
    install_name_tool -change $BREW_QT5_DIR/lib/QtCore.framework/Versions/5/QtCore \
        @executable_path/../Frameworks/QtCore.framework/Versions/5/QtCore \
        $PLATFORMS_DIR/libqcocoa.dylib
    install_name_tool -change $BREW_QT5_DIR/lib/QtGui.framework/Versions/5/QtGui \
        @executable_path/../Frameworks/QtGui.framework/Versions/5/QtGui \
        $PLATFORMS_DIR/libqcocoa.dylib
    install_name_tool -change $BREW_QT5_DIR/lib/QtWidgets.framework/Versions/5/QtWidgets \
        @executable_path/../Frameworks/QtWidgets.framework/Versions/5/QtWidgets \
        $PLATFORMS_DIR/libqcocoa.dylib

    cp src/autotype/mac/libkeepassx-autotype-cocoa.so $APP_DIR/Contents/MacOS/libkeepassx-autotype-cocoa.so

    # fix dependencies
    install_name_tool -change $QT5_DIR/lib/QtCore.framework/Versions/5/QtCore \
        @executable_path/../Frameworks/QtCore.framework/Versions/5/QtCore \
        $APP_DIR/Contents/MacOS/libkeepassx-autotype-cocoa.so
    install_name_tool -change $QT5_DIR/lib/QtGui.framework/Versions/5/QtGui \
        @executable_path/../Frameworks/QtGui.framework/Versions/5/QtGui \
        $APP_DIR/Contents/MacOS/libkeepassx-autotype-cocoa.so
    install_name_tool -change $QT5_DIR/lib/QtWidgets.framework/Versions/5/QtWidgets \
        @executable_path/../Frameworks/QtWidgets.framework/Versions/5/QtWidgets \
        $APP_DIR/Contents/MacOS/libkeepassx-autotype-cocoa.so
    install_name_tool -change $QT5_DIR/lib/QtConcurrent.framework/Versions/5/QtConcurrent \
        @executable_path/../Frameworks/QtConcurrent.framework/Versions/5/QtConcurrent \
        $APP_DIR/Contents/MacOS/libkeepassx-autotype-cocoa.so
    install_name_tool -change $LIBGCRYPT_DIR/lib/libgcrypt.20.dylib \
        @executable_path/../MacOS/libgcrypt.20.dylib \
        $APP_DIR/Contents/MacOS/libkeepassx-autotype-cocoa.so
fi
