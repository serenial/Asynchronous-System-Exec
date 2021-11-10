#!/bin/bash

# exit when any command fails
set -e

# keep track of the last executed command
trap 'last_command=$current_command; current_command=$BASH_COMMAND' DEBUG
# echo an error message before exiting
trap 'echo "\"${last_command}\" command filed with exit code $?."' EXIT

source $NILRT_SDK_CROSS_COMPILE_x64_SETUP

# The version of python supplied by the SDK and required for the build is broken but is first in the path - so delete it
rm $OECORE_NATIVE_SYSROOT/usr/bin/python*

## Build Boost ##
cd $BOOST_DIR \
    && echo '### Making boost ###' \
    && ./b2 toolset=gcc-x86_64_nilrt  cxxflags=-fPIC link=static runtime-link=static --disable-icu --prefix=$SDKTARGETSYSROOT/usr install > /dev/null

# Clean up
cd /
rm -rf $BOOST_DIR