#!/bin/bash

if cd build; then
    # Build
    PATH=$PATH:/mingw32/bin
    cmake .. -G "MSYS Makefiles" -DCMAKE_BUILD_TYPE=Release
    make -j4

    # Tests
    #make test CTEST_OUTPUT_ON_FAILURE=1 -j4

    # Zip package
    #make package
fi