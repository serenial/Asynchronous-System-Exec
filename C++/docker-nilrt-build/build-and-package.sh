#!/bin/bash
BUILDTYPE=$1;

shopt -s nocasematch

CMAKE_ARG=release

if [[ "$BUILDTYPE" = "debug" ]]; then
    echo "*** Building Debug ***"
    CMAKE_ARG=debug
else
    echo "*** Building Release *** - "
    echo "Supply argument \"debug\" to perform debug build"
fi

BUILD_ARCHIVE=/build-archive/build-$CMAKE_ARG.tar
BUILD_DIR=/build-$CMAKE_ARG

# make build dir
mkdir $BUILD_DIR

# attempt to extract any existing build-archive
echo "Attempting to extract build files from $BUILD_ARCHIVE"
tar -xf $BUILD_ARCHIVE

echo USING NI Linux RT Environment Setup Script at \"$NILRT_SDK_CROSS_COMPILE_x64_SETUP\"
echo USING OPKG Package Files at \"$OPKG_PKG_BUILD_DIR\"

# make sure files have unix line-endings
dos2unix $OPKG_PKG_BUILD_DIR/CONTROL/control
dos2unix $OPKG_PKG_BUILD_DIR/debian-binary

# this adds all the target's build tools into the path ahead of the hosts
source $NILRT_SDK_CROSS_COMPILE_x64_SETUP

BUILD_LIBRARY_NAME=serenial.io-ase.64.so
INSTALL_DIR_LIBRARY=/LabVIEW/serenial.io-ase/ase
INSTALL_DIR_TEST_BINARIES=/LabVIEW/test-binaries
INSTALL_DIR_PACKAGES=/LabVIEW/packages
BUILD_OUPUT_LIB_PATH=$INSTALL_DIR_LIBRARY/$BUILD_LIBRARY_NAME

# setup package build directory
OPKG_PACKAGE_BUILD_SYSTEM_LIB_DIR=$OPKG_PKG_BUILD_DIR/usr/lib
OPKG_PACKAGE_BUILD_BIN_DIR=$OPKG_PKG_BUILD_DIR/usr/bin
OPKG_PACKAGE_BUILD_LIB_DIR=$OPKG_PKG_BUILD_DIR/usr/lib

OPKG_LIBRARY_NAME=libserenial.io-ase
OPKG_LIBRARY_VER=$(awk '/Version: /{print $NF}' $OPKG_PKG_BUILD_DIR/CONTROL/control)
OPKG_LIBRARY_VER_MAJOR=$(echo ${OPKG_LIBRARY_VER%%.*})

echo USING OPKG Package File Version \"$OPKG_LIBRARY_VER\" - Major Version \"$OPKG_LIBRARY_VER_MAJOR\"

OPKG_PACKAGE_LIBRARY_PATH=$OPKG_PACKAGE_BUILD_LIB_DIR/$OPKG_LIBRARY_NAME.so.$OPKG_LIBRARY_VER
OPKG_PACKAGE_LIBRARY_LV_PATH=$OPKG_PACKAGE_BUILD_LIB_DIR/$OPKG_LIBRARY_NAME.so.$OPKG_LIBRARY_VER_MAJOR

mkdir -p $OPKG_PACKAGE_BUILD_BIN_DIR $OPKG_PACKAGE_BUILD_LIB_DIR $OPKG_PACKAGE_BUILD_SYSTEM_LIB_DIR $INSTALL_DIR_PACKAGES

#change to build dir
cd $BUILD_DIR

# configure, build, make and install

# cmake in path points to outdated version supplied with the nilrt toolchain
CMAKE=/usr/local/bin/cmake

# configure, build, archive build files and create opkg-package
# Add a relative symlink between the name that the LabVIEW library is expecting (serenial.io-ase.64.so)
# and the fully qualified library name (libserenial.io-ase.so.X.Y.Z)
$CMAKE -DLVCINTOOLS_INCLUDE_DIRS_64=/cintools \
 -DCMAKE_TOOLCHAIN_FILE=$OECORE_NATIVE_SYSROOT/usr/share/cmake/OEToolchainConfig.cmake \
 -DINSTALL_DIR_LIBRARY=$INSTALL_DIR_LIBRARY \
 -DINSTALL_DIR_TEST_BINARIES=$INSTALL_DIR_TEST_BINARIES \
 /C++ -G "Unix Makefiles" \
 && $CMAKE --build . --config $CMAKE_ARG \
 && $CMAKE --install . --config $CMAKE_ARG \
 && tar -cf $BUILD_ARCHIVE $BUILD_DIR \
 && echo "### Building Package ###" \
 && cp $BUILD_OUPUT_LIB_PATH $OPKG_PACKAGE_LIBRARY_PATH \
 && ln -sr $OPKG_PACKAGE_LIBRARY_PATH $OPKG_PACKAGE_LIBRARY_LV_PATH \
 && cp $INSTALL_DIR_TEST_BINARIES/ase-test-console-app-64 $OPKG_PACKAGE_BUILD_BIN_DIR \
 && opkg-build $OPKG_PKG_BUILD_DIR $INSTALL_DIR_PACKAGES