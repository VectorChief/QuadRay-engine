INC_PATH =                          \
        -I../core/config/

SRC_LIST =                          \
        simd_test.cpp

simd_test:
	arm-linux-gnueabi-g++ -O3 -g -static -march=armv7-a -marm \
        -DRT_LINUX -DRT_ARM -DRT_DEBUG=0 \
        ${INC_PATH} ${SRC_LIST} -o simd_test.arm

# Build target above is suitable for Maemo/N900 and DEB-based cross-compilation.
# Other (than DEB-based) Linux systems may have different name for the compiler.
# For native builds on ARMv7 (Raspberry Pi 2) use plain g++ reference.
