#!/bin/sh
# Intended for AArch64 macOS build environment with Apple Silicon (M1 chip)
# with Command Line Tools installed (macOS BigSur/Monterey tested)
# build on the least recent OS as binaries aren't always backward compatible

make -f simd_make_a64.mk macRM


make -f core_make_a64.mk macRM


# RooT demo compilation also requires XQuartz starting from version 2.8.0

cd ../root

make -f RooT_make_a64.mk macRM

cd ../test
