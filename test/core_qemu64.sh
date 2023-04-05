#!/bin/sh
# Intended for x86_64 Linux test environment
# with QEMU linux-user mode installed (64-bit Ubuntu MATE 20.04 LTS tested)
# run this script after bulid_cross.sh with 64-bit cross-compilers installed

# for fp64 version replace *f32 to *f64 below
# to change SIMD size-factor (n = 1, 2) use/add: -k n
# to change antialiasing mode (n = 2, 4) use/add: -a n
# to draw target-specific numbers in images use/add: -h


echo "========================================================"
echo "=== running core_qemu64 in background, check ../dump ==="
echo "=== wait for all 18 image-sets to be present: scr18* ==="
echo "=== use top to monitor when all qemu-* have finished ==="
echo "========================================================"

qemu-aarch64 -cpu cortex-a57 core_test.a64f32 -c 1 -o -l -i 3 &
qemu-aarch64 -cpu max,sve-max-vq=4 core_test.a64f32sve -c 1 -o -l -i 4 &

qemu-mips64el -cpu I6400 core_test.m64f32Lr6 -c 1 -o -l -i 5 &
qemu-mips64   -cpu I6400 core_test.m64f32Br6 -c 1 -o -l -i 6 &

# using -cpu power9 for power8 targets is a workaround for Ubuntu 22.04 LTS
# https://gcc.gnu.org/bugzilla/show_bug.cgi?id=109007

qemu-ppc64   -cpu POWER7 core_test.p64f32Bp7 -c 1 -o -l -i 7 &
qemu-ppc64le -cpu POWER9 core_test.p64f32Lp8 -c 1 -o -l -i 8 &
qemu-ppc64le -cpu POWER9 core_test.p64f32Lp9 -c 1 -o -l -i 9 &


