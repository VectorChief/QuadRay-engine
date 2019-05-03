#!/bin/sh
# Intended for x86_64 Linux build environment
# with native g++ multilib-compiler installed (64-bit Linux Mint 18 tested)

make -f simd_make_x86.mk build -j4
make -f simd_make_x32.mk build

make -f simd_make_x86.mk strip
make -f simd_make_x32.mk strip


make -f core_make_x86.mk build
make -f core_make_x32.mk build

make -f core_make_x86.mk strip
make -f core_make_x32.mk strip


# RooT demo compilation requires Xext development library in addition to g++

cd ../root

make -f RooT_make_x86.mk build

make -f RooT_make_x86.mk strip

cd ../test
