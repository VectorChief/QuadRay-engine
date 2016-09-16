#!/bin/sh
# Intended for ARMv7 Linux build environment
# with native g++ compiler installed (32-bit Raspbian 7 and 8 tested)

make -f simd_make_arm.mk clean_rpiX


make -f core_make_arm.mk clean_rpiX


cd ../

make -f RooT_make_arm.mk clean_rpiX

cd test
