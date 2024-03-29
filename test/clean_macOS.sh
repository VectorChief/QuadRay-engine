#!/bin/sh
# Intended for x86_64 Mac OS X / OS X / macOS build environment
# with Command Line Tools installed (Mac OS X Lion / macOS High Sierra tested)
# build on the least recent OS as binaries aren't always backward compatible

make -f simd_make_x64.mk macRM


make -f core_make_x64.mk macRM


# RooT demo compilation also requires XQuartz starting from OS X Mountain Lion

cd ../root

make -f RooT_make_x64.mk macRM

cd ../test
