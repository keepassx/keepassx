#!/bin/bash

cd build

# Cleanup
rm src/KeePassX.exe
rm src/autotype/windows/libkeepassx-autotype-windows.dll

# Build
PATH=$PATH:/mingw32/bin
cmake .. -G "MSYS Makefiles" -DCMAKE_BUILD_TYPE=Release
make -j4

# Tests
#make test CTEST_OUTPUT_ON_FAILURE=1 -j4

cp src/KeePassX.exe /mingw32/bin/KeePassX.exe
cp src/autotype/windows/libkeepassx-autotype-windows.dll /mingw32/bin/libkeepassx-autotype-windows.dll