
INC_PATH =                                  \
        -I../core/config/                   \
        -I../core/engine/                   \
        -I../core/system/                   \
        -I../core/tracer/                   \
        -I../data/materials/                \
        -I../data/objects/                  \
        -I../data/scenes/                   \
        -I../data/textures/

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
        RooT_linux.cpp

LIB_PATH =

LIB_LIST =                                  \
        -lm                                 \
        -lstdc++                            \
        -lX11                               \
        -lXext                              \
        -lpthread


build: RooT_a32

strip:
	strip RooT.a32*

clean:
	rm RooT.a32*


RooT_a32:
	g++ -O3 -g -pthread -mabi=ilp32 \
        -DRT_LINUX -DRT_A32 -DRT_128=1 -DRT_256=1 \
        -DRT_POINTER=32 -DRT_ADDRESS=32 -DRT_ELEMENT=32 -DRT_ENDIAN=0 \
        -DRT_DEBUG=0 -DRT_PATH="../" -DRT_FULLSCREEN=0 \
        -DRT_EMBED_STDOUT=0 -DRT_EMBED_FILEIO=0 -DRT_EMBED_TEX=1 \
        ${INC_PATH} ${SRC_LIST} ${LIB_PATH} ${LIB_LIST} -o RooT.a32


clang: RooT.a32

RooT.a32:
	clang++ -O3 -g -pthread -mabi=ilp32 \
        -DRT_LINUX -DRT_A32 -DRT_128=1 -DRT_256=1 \
        -DRT_POINTER=32 -DRT_ADDRESS=32 -DRT_ELEMENT=32 -DRT_ENDIAN=0 \
        -DRT_DEBUG=0 -DRT_PATH="../" -DRT_FULLSCREEN=0 \
        -DRT_EMBED_STDOUT=0 -DRT_EMBED_FILEIO=0 -DRT_EMBED_TEX=1 \
        ${INC_PATH} ${SRC_LIST} ${LIB_PATH} ${LIB_LIST} -o RooT.a32


# Prerequisites for the build:
# multilib-compiler for AArch64 is installed and in the PATH variable,
# sudo apt-get update (on AArch64 host or QEMU system mode)
# sudo apt-get install make g++-multilib (plus X11/Xext libs for ILP32 ABI)
# (recent upstream g++-5-aarch64 series may not fully support ILP32 ABI)
#
# Building/running RooT demo:
# make -f RooT_make_a32.mk
# ./RooT.a32 (on AArch64 host or QEMU system mode with ILP32 X11/Xext libs)
# (hasn't been verified yet due to lack of available libs)

# Clang native build should theoretically work too (not tested):
# sudo apt-get update (on Ubuntu add "universe" to "main" /etc/apt/sources.list)
# sudo apt-get install clang g++-multilib (plus X11/Xext libs for ILP32 ABI)
# make -f RooT_make_a32.mk clean
# make -f RooT_make_a32.mk clang
# (hasn't been verified yet due to lack of available libs)

# RooT demo uses runtime SIMD target selection, multiple can be specified above
# on RISC targets top value above is chosen by default, use -n/-k/-s to override
# 256-bit SIMD is achieved by combining pairs of 128-bit registers/instructions

# For interpretation of SIMD build flags check compatibility layer in rtzero.h
# or refer to the corresponding simd_make_***.mk file.

# 32-bit ABI hasn't been fully tested yet due to lack of available libs,
# check out 64/32-bit (ptr/adr) hybrid mode for 64-bit ABI in RooT_make_a64.mk
