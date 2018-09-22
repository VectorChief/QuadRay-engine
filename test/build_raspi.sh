#!/bin/sh
# Intended for ARMv7 Linux build environment
# with native g++ compiler installed (32-bit Raspbian 7 and 8 tested)

make -f simd_make_arm.mk build_rpiX -j8

make -f simd_make_arm.mk strip_rpiX


make -f core_make_arm.mk build_rpiX -j8

make -f core_make_arm.mk strip_rpiX


cd ../root

make -f RooT_make_arm.mk build_rpiX -j8

make -f RooT_make_arm.mk strip_rpiX

cd ../test
