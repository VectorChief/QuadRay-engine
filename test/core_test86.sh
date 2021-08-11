#!/bin/sh
# Intended for x86_64 Linux test environment
# with multilib capabilities (64-bit Linux Mint 18 tested)
# run this script after bulid_multi.sh with multilib-compiler installed

# run before core_test64.sh to check all 18 image-sets
# run core_test64.sh after to compare results in place
# to change antialiasing mode (n = 2, 4) use/add: -a n
# to draw target-specific numbers in images use/add: -h


echo "========================================================"
echo "=== running core_test86 in background, check ../dump ==="
echo "=== wait for all 18 image-sets to be present: scr18* ==="
echo "=== use top to monitor when all core_* have finished ==="
echo "========================================================"

./core_test.x86 -n 1 -c 1 -o -l -i 0 &
./core_test.x86 -n 2 -c 1 -o -l -i 1 &
./core_test.x86 -n 4 -c 1 -o -l -i 2 &
./core_test.x32 -c 1 -o -l -i 3 &


