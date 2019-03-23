#!/bin/bash

# Install xcode
# Install homebrew
# > brew install cmake qt5 libgcrypt

if cd build; then
    QT5_DIR="/usr/local/opt/qt5"

    export CMAKE_PREFIX_PATH=$QT5_DIR
    cmake .. -DCMAKE_INSTALL_PREFIX=/usr/local -DCMAKE_BUILD_TYPE=Release
    make -j4

    $QT5_DIR/bin/macdeployqt src/KeePassX.app
fi
