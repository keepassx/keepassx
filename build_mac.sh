#!/bin/bash

# Install xcode
# Install homebrew
# > brew install cmake qt5 libgcrypt

if cd build; then
    export CMAKE_PREFIX_PATH=/usr/local/opt/qt5/
    cmake .. -DCMAKE_BUILD_TYPE=Release
    make -j4
fi
