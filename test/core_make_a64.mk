
INC_PATH =                                  \
        -I../core/config/                   \
        -I../core/engine/                   \
        -I../core/system/                   \
        -I../core/tracer/                   \
        -I../data/materials/                \
        -I../data/objects/                  \
        -I../data/textures/                 \
        -Iscenes/

SRC_LIST =                                  \
        ../core/engine/engine.cpp           \
        ../core/engine/object.cpp           \
        ../core/engine/rtgeom.cpp           \
        ../core/engine/rtimag.cpp           \
        ../core/system/system.cpp           \
        ../core/tracer/tracer.cpp           \
        ../core/tracer/tracer_128v1.cpp     \
        ../core/tracer/tracer_128v4.cpp     \
        ../core/tracer/tracer_256v1.cpp     \
        core_test.cpp

LIB_PATH =

LIB_LIST =                                  \
        -lm                                 \
        -lstdc++


build: core_test_a64_32 core_test_a64_64 core_test_a64f32 core_test_a64f64

strip:
	aarch64-linux-gnu-strip core_test.a64*

clean:
	rm core_test.a64*


core_test_a64_32:
	aarch64-linux-gnu-g++ -O3 -g -static \
        -DRT_LINUX -DRT_A64 -DRT_128=1 -DRT_256=1 \
        -DRT_POINTER=64 -DRT_ADDRESS=32 -DRT_ELEMENT=32 -DRT_ENDIAN=0 \
        -DRT_DEBUG=0 -DRT_PATH="../" \
        -DRT_EMBED_STDOUT=0 -DRT_EMBED_FILEIO=0 -DRT_EMBED_TEX=1 \
        ${INC_PATH} ${SRC_LIST} ${LIB_PATH} ${LIB_LIST} -o core_test.a64_32

core_test_a64_64:
	aarch64-linux-gnu-g++ -O3 -g -static \
        -DRT_LINUX -DRT_A64 -DRT_128=1 -DRT_256=1 \
        -DRT_POINTER=64 -DRT_ADDRESS=32 -DRT_ELEMENT=64 -DRT_ENDIAN=0 \
        -DRT_DEBUG=0 -DRT_PATH="../" \
        -DRT_EMBED_STDOUT=0 -DRT_EMBED_FILEIO=0 -DRT_EMBED_TEX=1 \
        ${INC_PATH} ${SRC_LIST} ${LIB_PATH} ${LIB_LIST} -o core_test.a64_64

core_test_a64f32:
	aarch64-linux-gnu-g++ -O3 -g -static \
        -DRT_LINUX -DRT_A64 -DRT_128=1 -DRT_256=1 \
        -DRT_POINTER=64 -DRT_ADDRESS=64 -DRT_ELEMENT=32 -DRT_ENDIAN=0 \
        -DRT_DEBUG=0 -DRT_PATH="../" \
        -DRT_EMBED_STDOUT=0 -DRT_EMBED_FILEIO=0 -DRT_EMBED_TEX=1 \
        ${INC_PATH} ${SRC_LIST} ${LIB_PATH} ${LIB_LIST} -o core_test.a64f32

core_test_a64f64:
	aarch64-linux-gnu-g++ -O3 -g -static \
        -DRT_LINUX -DRT_A64 -DRT_128=1 -DRT_256=1 \
        -DRT_POINTER=64 -DRT_ADDRESS=64 -DRT_ELEMENT=64 -DRT_ENDIAN=0 \
        -DRT_DEBUG=0 -DRT_PATH="../" \
        -DRT_EMBED_STDOUT=0 -DRT_EMBED_FILEIO=0 -DRT_EMBED_TEX=1 \
        ${INC_PATH} ${SRC_LIST} ${LIB_PATH} ${LIB_LIST} -o core_test.a64f64


# On Ubuntu 16.04 Live DVD add "universe multiverse" to "main restricted"
# in /etc/apt/sources.list (sudo gedit /etc/apt/sources.list) then run:
# sudo apt-get update (ignoring the old database errors in the end)
#
# Prerequisites for the build:
# (cross-)compiler for AArch64 is installed and in the PATH variable.
# sudo apt-get install g++-aarch64-linux-gnu
#
# Prerequisites for emulation:
# recent QEMU(-2.5) is installed or built from source and in the PATH variable.
# sudo apt-get install qemu
#
# Building/running CORE test:
# make -f core_make_a64.mk
# qemu-aarch64 -cpu cortex-a57 core_test.a64f32 -i -a
# (should produce antialiased (-a) images (-i) in the ../dump subfolder)

# Clang compilation works too (takes much longer prior to 3.8), use (replace):
# clang++ -Wno-logical-op-parentheses -Wno-bitwise-op-parentheses
# sudo apt-get install clang

# core_test uses runtime SIMD target selection, multiple can be specified above
# on RISC targets top value above is chosen by default, use -n/-k/-s to override
# 256-bit SIMD is achieved by combining pairs of 128-bit registers/instructions

# For interpretation of SIMD build flags check compatibility layer in rtzero.h

# 64/32-bit (ptr/adr) hybrid mode is compatible with native 64-bit ABI,
# use (replace): RT_ADDRESS=32, rename the binary to core_test.a64_**

# 64-bit packed SIMD mode (fp64/int64) is supported on 64-bit targets,
# use (replace): RT_ELEMENT=64, rename the binary to core_test.a64*64
