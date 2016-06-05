
INC_PATH =                              \
        -I../core/config/               \
        -I../core/engine/               \
        -I../core/system/               \
        -I../core/tracer/               \
        -I../data/materials/            \
        -I../data/objects/              \
        -I../data/textures/             \
        -Iscenes/

SRC_LIST =                              \
        ../core/engine/engine.cpp       \
        ../core/engine/object.cpp       \
        ../core/engine/rtgeom.cpp       \
        ../core/engine/rtimag.cpp       \
        ../core/system/system.cpp       \
        ../core/tracer/tracer.cpp       \
        ../core/tracer/tracer_128v1.cpp \
        core_test.cpp

LIB_PATH =

LIB_LIST =                              \
        -lm                             \
        -lstdc++

core_test:
	aarch64-linux-gnu-g++ -O3 -g -static \
        -DRT_LINUX -DRT_A64 -DRT_128=1 \
        -DRT_POINTER=64 -DRT_ADDRESS=32 -DRT_ENDIAN=0 \
        -DRT_DEBUG=0 -DRT_PATH="../" \
        -DRT_EMBED_STDOUT=0 -DRT_EMBED_FILEIO=0 -DRT_EMBED_TEX=1 \
        ${INC_PATH} ${SRC_LIST} ${LIB_PATH} ${LIB_LIST} -o core_test.a64_32

# Prerequisites for the build:
# (cross-)compiler for AArch64 is installed and in the PATH variable.
# Prerequisites for emulation:
# latest QEMU(-2.5) is built from source and in the PATH variable.
#
# make -f core_make_a32.mk
# qemu-aarch64 -cpu cortex-a57 core_test.a64_32 -i -a
# (should produce antialiased (-a) images (-i) in the ../dump subfolder)

# For actual a32 target use: -mabi=ilp32 -DRT_A32 -DRT_POINTER=32 (replace),
# rename produced binary to core_test.a32 or adjust the build command above.
# Experimental 64/32-bit hybrid support is enabled by default for compatibility
# with wider spectrum of toolchains/libraries in the standard a64 target.
