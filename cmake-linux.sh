#!/bin/bash
#
# Description
#
#     Use cmake to generate a GNU Make build. The output of this script
#     will be Makefiles that can be used to build on a Linux platform.
#     Output will not be generated in the source code tree, but rather
#     in the subdirectory: build_<config>.
#
# Usage
#
#     ./cmake-linux.sh [cmake-build-type]
#
################################################################################

set -e

DIR=$(cd `dirname $0` && pwd) # Get directory of this script
BUILD_CONFIG=${1:-Debug} # Use Debug if no arg provided

function usage()
{
cat <<EOF
USAGE:
    $0 [cmake-build-type]

    cmake-build-type

        The build configuration type.

        One of:
            Debug                Compile with -g
            Release              Compile -O3 -DNDEBUG
            RelWithDebInfo       Compile release with debug info
            MinSizeRel           Compile size-optimized release
EOF
}

case $BUILD_CONFIG in
    Debug|Release|RelWithDebInfo|MinSizeRel)
        ;;
    *)
        echo "Invalid build type!"
        usage
        exit 1
        ;;
esac

BUILD_DIR=build_${BUILD_CONFIG,,} # Use parameter expansion: to-lowercase

cd $DIR # Make sure working directory is script directory
if [ ! -d $BUILD_DIR ]; then
    mkdir $BUILD_DIR
fi

cd $BUILD_DIR
: ${CC:=/usr/bin/gcc-8}
: ${CXX:=/usr/bin/g++-8}
export CC CXX
cmake \
    -G "Unix Makefiles" \
    -Wno-dev \
    -DCMAKE_BUILD_TYPE=${BUILD_CONFIG} \
    -DOpenCV_ROOT="/home/vagrant/uls/lsm/lsm-10/third-party/install" \
    -DENABLE_UVC_DEBUGGING=ON \
    ..

exit 0
