#!/bin/sh
# Intended for x86_64 Linux build environment
# with native g++ compiler installed (64-bit Linux Mint 18 tested)

make -f simd_make_x64.mk build -j8

make -f simd_make_x64.mk strip


make -f core_make_x64.mk build -j4

make -f core_make_x64.mk strip


# RooT demo compilation requires Xext development library in addition to g++

cd ../root

make -f RooT_make_x64.mk build -j4

make -f RooT_make_x64.mk strip

cd ../test
