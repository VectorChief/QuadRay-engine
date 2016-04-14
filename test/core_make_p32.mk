
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
        ../core/tracer/tracer_128v2.cpp \
        core_test.cpp

LIB_PATH =

LIB_LIST =                              \
        -lm                             \
        -lstdc++

core_test:
	powerpc-linux-gnu-g++ -O3 -g -static \
        -DRT_LINUX -DRT_P32 -DRT_128=1 \
        -DRT_POINTER=32 -DRT_ADDRESS=32 -DRT_ENDIAN=1 \
        -DRT_DEBUG=1 -DRT_PATH="../" \
        -DRT_EMBED_STDOUT=0 -DRT_EMBED_FILEIO=0 -DRT_EMBED_TEX=1 \
        ${INC_PATH} ${SRC_LIST} ${LIB_PATH} ${LIB_LIST} -o core_test.p32

# Prerequisites for the build:
# (cross-)compiler for PowerPC is installed and in the PATH variable.
# Prerequisites for emulation:
# latest QEMU(-2.5) is built from source and in the PATH variable.
#
# make -f core_make_p32.mk
# qemu-ppc -cpu G4 core_test.p32 -i -a
# (should produce antialiased (-a) images (-i) in the ../dump subfolder)

# For 32-bit Power(7,7+,8) VSX target use (replace): -DRT_128=2
# qemu-ppc64abi32 -cpu POWER7 core_test.p32 -i -a
# (should produce antialiased (-a) images (-i) in the ../dump subfolder)
