#!/bin/sh
# Intended for x86_64 Linux test environment
# with QEMU linux-user mode installed (64-bit Ubuntu MATE 20.04 LTS tested)
# run this script after bulid_cross.sh with 32-bit cross-compilers installed

# run before core_qemu64.sh to check all 18 image-sets
# run after core_qemu64.sh to compare results in place
# to change antialiasing mode (n = 2, 4) use/add: -a n
# to draw target-specific numbers in images use/add: -h


echo "========================================================"
echo "=== running core_qemu32 in background, check ../dump ==="
echo "=== wait for all 18 image-sets to be present: scr18* ==="
echo "=== use top to monitor when all qemu-* have finished ==="
echo "========================================================"

qemu-arm -cpu cortex-a8  core_test.arm_v1 -c 1 -o -l -i 1 &
qemu-arm -cpu cortex-a15 core_test.arm_v2 -c 1 -o -l -i 2 &

qemu-mipsel -cpu P5600 core_test.m32Lr5 -c 1 -o -l -i 5 &
qemu-mips   -cpu P5600 core_test.m32Br5 -c 1 -o -l -i 6 &

# ppc64abi32 targets are deprecated since QEMU 5.2.0 (dropped in Ubuntu 22.04)

qemu-ppc        -cpu G4     core_test.p32Bg4 -c 1 -o -l -i 4 &
#qemu-ppc64abi32 -cpu POWER7 core_test.p32Bp7 -c 1 -o -l -i 7 &
#qemu-ppc64abi32 -cpu POWER8 core_test.p32Bp8 -c 1 -o -l -i 8 &
#qemu-ppc64abi32 -cpu POWER9 core_test.p32Bp9 -c 1 -o -l -i 9 &


