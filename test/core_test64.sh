#!/bin/sh
# Intended for x86_64 Linux test environment
# tested on 64-bit Linux Mint 18, 64-bit Ubuntu MATE 18.04/20.04 LTS
# run this script after bulid_linux.sh with native compiler installed

# for fp64 version replace *f32 to *f64 below
# to change SIMD size-factor (n = 1, 2) use/add: -k n
# to change antialiasing mode (n = 2, 4) use/add: -a n
# to draw target-specific numbers in images use/add: -h


echo "========================================================"
echo "=== running core_test64 in background, check ../dump ==="
echo "=== wait for all 18 image-sets to be present: scr18* ==="
echo "=== use top to monitor when all core_* have finished ==="
echo "========================================================"

./core_test.x64f32 -n 1 -c 1 -o -l -i 0 &
./core_test.x64f32 -n 2 -c 1 -o -l -i 1 &
./core_test.x64f32 -n 4 -c 1 -o -l -i 2 &


