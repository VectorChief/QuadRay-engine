
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
        ../core/tracer/tracer_256v1.cpp \
        core_test.cpp

LIB_PATH =

LIB_LIST =                              \
        -lm                             \
        -lstdc++


build: core_test_a32

strip:
	aarch64-linux-gnu-strip core_test.a32

clean:
	rm core_test.a32


core_test_a32:
	aarch64-linux-gnu-g++ -O3 -g -static -mabi=ilp32 \
        -DRT_LINUX -DRT_A32 -DRT_128=1 -DRT_256=1 \
        -DRT_POINTER=32 -DRT_ADDRESS=32 -DRT_ELEMENT=32 -DRT_ENDIAN=0 \
        -DRT_DEBUG=0 -DRT_PATH="../" \
        -DRT_EMBED_STDOUT=0 -DRT_EMBED_FILEIO=0 -DRT_EMBED_TEX=1 \
        ${INC_PATH} ${SRC_LIST} ${LIB_PATH} ${LIB_LIST} -o core_test.a32


# On Ubuntu 16.04 Live CD add "universe multiverse" to "main restricted"
# in /etc/apt/sources.list (sudo gedit /etc/apt/sources.list) then run:
# sudo apt-get update (ignoring the old database errors in the end)
#
# Prerequisites for the build:
# (cross-)compiler for AArch64 is installed and in the PATH variable.
# sudo apt-get install g++-aarch64-linux-gnu
# (recent upstream g++-5-aarch64 series may not fully support ILP32 ABI)
#
# Building/running CORE test:
# make -f core_make_a32.mk
# (recent upstream QEMU(-2.6) doesn't currently support AArch64 ILP32 ABI)

# core_test uses runtime SIMD target selection, multiple can be specified above
# on RISC targets top value above is chosen by default, use -k/-n/-s to override
# 256-bit SIMD is achieved by combining pairs of 128-bit registers/instructions

# 32-bit ABI hasn't been fully tested yet due to lack of available libs,
# check out 64/32-bit (ptr/adr) hybrid mode for 64-bit ABI in core_make_a64.mk
