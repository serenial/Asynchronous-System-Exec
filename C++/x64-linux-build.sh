#!/bin/bash

VCPKGROOT=~/vcpkg

BUILDTYPE=$1;

shopt -s nocasematch

BUILD_TYPE=release

if [[ "$BUILDTYPE" = "debug" ]]; then
    echo "*** Building Debug ***"
    BUILD_TYPE=debug
else
    echo "*** Building Release *** - "
    echo "Supply argument \"debug\" to perform debug build"
fi

SCRIPT_DIR=$( cd -- "$( dirname -- "${BASH_SOURCE[0]}" )" &> /dev/null && pwd )
BUILD_DIR=$SCRIPT_DIR/build

mkdir -p $BUILD_DIR
cd $BUILD_DIR

cmake -DCMAKE_TOOLCHAIN_FILE:STRING=$VCPKGROOT/scripts/buildsystems/vcpkg.cmake \
-DCMAKE_C_COMPILER:FILEPATH=/usr/bin/gcc-7 \
-DCMAKE_CXX_COMPILER:FILEPATH=/usr/bin/g++-7 \
-DCMAKE_BUILD_TYPE:STRING=$BUILD_TYPE \
-DINSTALL_DIR_LIBRARY:STRING=$SCRIPT_DIR/../LabVIEW/serenial.io-ase/ase/ \
-DINSTALL_DIR_TEST_BINARIES:STRING=$SCRIPT_DIR/../LabVIEW/test-binaries/ \
../ -G "Unix Makefiles" \
 && cmake --build . --config $BUILD_TYPE \
 && sudo cmake --install . --config $BUILD_TYPE