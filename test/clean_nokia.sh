#!/bin/sh
# Intended for scratchbox Linux build environment (32-bit Ubuntu 10.10 tested)
# http://wiki.maemo.org/Documentation/Maemo_5_Final_SDK_Installation

make -f simd_make_arm.mk clean_n900


make -f core_make_arm.mk clean_n900


cd ../

make -f RooT_make_arm.mk clean_n900

cd test
