#!/bin/sh
# Intended for x86_64 Mac OS X / OS X / macOS build environment
# with Command Line Tools installed (Mac OS X Lion / macOS High Sierra tested)

make -f simd_make_x64.mk build -j8

make -f simd_make_x64.mk macRD

make -f simd_make_x64.mk strip

make -f simd_make_x64.mk macOS


make -f core_make_x64.mk build -j4

make -f core_make_x64.mk macRD

make -f core_make_x64.mk strip

make -f core_make_x64.mk macOS


# RooT demo compilation also requires XQuartz starting from OS X Mountain Lion

cd ../root

make -f RooT_make_x64.mk build -j4

make -f RooT_make_x64.mk macRD

make -f RooT_make_x64.mk strip

make -f RooT_make_x64.mk macOS

cd ../test
