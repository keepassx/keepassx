#!/bin/bash

# Install xcode
# Install homebrew
# > brew install cmake qt5 libgcrypt

if cd build; then
    QT5_DIR="/usr/local/opt/qt5"
    LIBGCRYPT_DIR="/usr/local/opt/libgcrypt"

    export CMAKE_PREFIX_PATH=$QT5_DIR
    cmake .. -DCMAKE_INSTALL_PREFIX=/usr/local -DCMAKE_BUILD_TYPE=Release
    make -j4

    $QT5_DIR/bin/macdeployqt src/KeePassX.app

    # fix dependencies
    install_name_tool -change $QT5_DIR/lib/QtCore.framework/Versions/5/QtCore \
        @executable_path/../Frameworks/QtCore.framework/Versions/5/QtCore \
        src/autotype/mac/libkeepassx-autotype-cocoa.so
    install_name_tool -change $QT5_DIR/lib/QtGui.framework/Versions/5/QtGui \
        @executable_path/../Frameworks/QtGui.framework/Versions/5/QtGui \
        src/autotype/mac/libkeepassx-autotype-cocoa.so
    install_name_tool -change $QT5_DIR/lib/QtWidgets.framework/Versions/5/QtWidgets \
        @executable_path/../Frameworks/QtWidgets.framework/Versions/5/QtWidgets \
        src/autotype/mac/libkeepassx-autotype-cocoa.so
    install_name_tool -change $QT5_DIR/lib/QtConcurrent.framework/Versions/5/QtConcurrent \
        @executable_path/../Frameworks/QtConcurrent.framework/Versions/5/QtConcurrent \
        src/autotype/mac/libkeepassx-autotype-cocoa.so
    install_name_tool -change $LIBGCRYPT_DIR/lib/libgcrypt.20.dylib \
        @executable_path/../MacOS/libgcrypt.20.dylib \
        src/autotype/mac/libkeepassx-autotype-cocoa.so
fi
