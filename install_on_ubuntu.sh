#!/bin/bash

# Convenience script to install and launch KeePassX
# Tested on Ubuntu 14.14

# Install depencencies
# Qt4
sudo apt-get install qt4-dev-tools libqt4-dev libqt4-core libqt4-gui
# GCrypt
sudo apt-get install libgcrypt11-dev zlib1g-dev
# cmake
sudo apt-get install cmake

# Build
mkdir build
cd build
cmake .. -DCMAKE_INSTALL_PREFIX=/usr/local -DCMAKE_BUILD_TYPE=Release

# Install
sudo make install

# Launch
/usr/local/bin/keepassx &
