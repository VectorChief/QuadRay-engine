#!/bin/sh
# Intended for x86_64 Linux build environment
# with native g++ compiler installed (64-bit Linux Mint 18 tested)

make -f simd_make_x64.mk clean


make -f core_make_x64.mk clean


# RooT demo compilation requires Xext development library in addition to g++

cd ../root

make -f RooT_make_x64.mk clean

cd ../test
