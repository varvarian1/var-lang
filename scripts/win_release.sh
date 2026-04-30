#!/bin/bash

# build for Windows

mkdir -p build-win
cd build-win

cmake .. -DCMAKE_TOOLCHAIN_FILE=../cmake/win-toolchain.cmake
make

echo "Release created for Windows!"