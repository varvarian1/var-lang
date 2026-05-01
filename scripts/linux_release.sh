#!/bin/bash

# build for Linux

mkdir -p build-linux
cd build-linux

cmake .. -DCMAKE_TOOLCHAIN_FILE=../cmake/linux-toolchain.cmake
cmake --build . --parallel $(nproc)

echo "Release created for Linux!"