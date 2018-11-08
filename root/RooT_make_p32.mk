
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
        ../core/tracer/tracer_128v4.cpp     \
        ../core/tracer/tracer_256v4_r8.cpp  \
        RooT_linux.cpp

LIB_PATH =

LIB_LIST =                                  \
        -lm                                 \
        -lstdc++                            \
        -lX11                               \
        -lXext                              \
        -lpthread


build: RooT_p32Bg4

strip:
	strip RooT.p32*

clean:
	rm RooT.p32*


RooT_p32Bg4:
	g++ -O3 -g -pthread \
        -DRT_LINUX -DRT_P32 -DRT_128=4 -DRT_256_R8=4 \
        -DRT_POINTER=32 -DRT_ADDRESS=32 -DRT_ELEMENT=32 -DRT_ENDIAN=1 \
        -DRT_DEBUG=0 -DRT_PATH="../" -DRT_FULLSCREEN=0 \
        -DRT_EMBED_STDOUT=0 -DRT_EMBED_FILEIO=0 -DRT_EMBED_TEX=1 \
        ${INC_PATH} ${SRC_LIST} ${LIB_PATH} ${LIB_LIST} -o RooT.p32Bg4


clang: RooT.p32Bg4

RooT.p32Bg4:
	clang++ -O3 -g -pthread \
        -DRT_LINUX -DRT_P32 -DRT_128=4 -DRT_256_R8=4 \
        -DRT_POINTER=32 -DRT_ADDRESS=32 -DRT_ELEMENT=32 -DRT_ENDIAN=1 \
        -DRT_DEBUG=0 -DRT_PATH="../" -DRT_FULLSCREEN=0 \
        -DRT_EMBED_STDOUT=0 -DRT_EMBED_FILEIO=0 -DRT_EMBED_TEX=1 \
        ${INC_PATH} ${SRC_LIST} ${LIB_PATH} ${LIB_LIST} -o RooT.p32Bg4


# Prerequisites for the build:
# native-compiler for PowerPC is installed and in the PATH variable.
# sudo apt-get install g++ libxext-dev (on G4 host or QEMU system mode)
# (legacy 32-bit powerpc target may no longer be supported on latest distros)
#
# Building/running RooT demo:
# make -f RooT_make_p32.mk
# ./RooT.p32Bg4 (on G4 host or QEMU system mode)
# (hasn't been verified yet due to lack of target host system)
# (SIMD and CORE tests pass in QEMU linux-user mode, check test subfolder)

# RooT demo uses runtime SIMD target selection, multiple can be specified above
# on RISC targets top value above is chosen by default, use -n/-k/-s to override
# 256-bit SIMD is achieved by combining pairs of 128-bit registers/instructions

# For interpretation of SIMD build flags check compatibility layer in rtzero.h
