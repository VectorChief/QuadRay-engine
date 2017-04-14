
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
        ../core/tracer/tracer_256v1.cpp     \
        RooT_linux.cpp

LIB_PATH =

LIB_LIST =                                  \
        -lm                                 \
        -lstdc++                            \
        -lX11                               \
        -lXext                              \
        -lpthread


build: RooT_a64_32 RooT_a64_64 RooT_a64f32 RooT_a64f64

strip:
	aarch64-linux-gnu-strip RooT.a64*

clean:
	rm RooT.a64*


RooT_a64_32:
	aarch64-linux-gnu-g++ -O3 -g -pthread \
        -DRT_LINUX -DRT_A64 -DRT_128=1 -DRT_256=1 \
        -DRT_POINTER=64 -DRT_ADDRESS=32 -DRT_ELEMENT=32 -DRT_ENDIAN=0 \
        -DRT_DEBUG=0 -DRT_PATH="../" -DRT_FULLSCREEN=0 \
        -DRT_EMBED_STDOUT=0 -DRT_EMBED_FILEIO=0 -DRT_EMBED_TEX=1 \
        ${INC_PATH} ${SRC_LIST} ${LIB_PATH} ${LIB_LIST} -o RooT.a64_32

RooT_a64_64:
	aarch64-linux-gnu-g++ -O3 -g -pthread \
        -DRT_LINUX -DRT_A64 -DRT_128=1 -DRT_256=1 \
        -DRT_POINTER=64 -DRT_ADDRESS=32 -DRT_ELEMENT=64 -DRT_ENDIAN=0 \
        -DRT_DEBUG=0 -DRT_PATH="../" -DRT_FULLSCREEN=0 \
        -DRT_EMBED_STDOUT=0 -DRT_EMBED_FILEIO=0 -DRT_EMBED_TEX=1 \
        ${INC_PATH} ${SRC_LIST} ${LIB_PATH} ${LIB_LIST} -o RooT.a64_64

RooT_a64f32:
	aarch64-linux-gnu-g++ -O3 -g -pthread \
        -DRT_LINUX -DRT_A64 -DRT_128=1 -DRT_256=1 \
        -DRT_POINTER=64 -DRT_ADDRESS=64 -DRT_ELEMENT=32 -DRT_ENDIAN=0 \
        -DRT_DEBUG=0 -DRT_PATH="../" -DRT_FULLSCREEN=0 \
        -DRT_EMBED_STDOUT=0 -DRT_EMBED_FILEIO=0 -DRT_EMBED_TEX=1 \
        ${INC_PATH} ${SRC_LIST} ${LIB_PATH} ${LIB_LIST} -o RooT.a64f32

RooT_a64f64:
	aarch64-linux-gnu-g++ -O3 -g -pthread \
        -DRT_LINUX -DRT_A64 -DRT_128=1 -DRT_256=1 \
        -DRT_POINTER=64 -DRT_ADDRESS=64 -DRT_ELEMENT=64 -DRT_ENDIAN=0 \
        -DRT_DEBUG=0 -DRT_PATH="../" -DRT_FULLSCREEN=0 \
        -DRT_EMBED_STDOUT=0 -DRT_EMBED_FILEIO=0 -DRT_EMBED_TEX=1 \
        ${INC_PATH} ${SRC_LIST} ${LIB_PATH} ${LIB_LIST} -o RooT.a64f64


# Prerequisites for the build:
# native-compiler for AArch64 is installed and in the PATH variable.
# sudo apt-get install g++ libxext-dev (on AArch64 host or QEMU system mode)
#
# Building/running RooT demo:
# make -f RooT_make_a64.mk
# ./RooT.a64f32 (on AArch64 host or QEMU system mode)
# (hasn't been verified due to lack of target host system)
# (SIMD and CORE tests pass in QEMU linux-user mode, check test subfolder)

# Clang compilation works too (takes much longer prior to 3.8), use (replace):
# clang++ -Wno-logical-op-parentheses -Wno-bitwise-op-parentheses
# sudo apt-get install clang

# RooT demo uses runtime SIMD target selection, multiple can be specified above
# on RISC targets top value above is chosen by default, use -n/-k/-s to override
# 256-bit SIMD is achieved by combining pairs of 128-bit registers/instructions

# For interpretation of SIMD build flags check compatibility layer in rtzero.h

# 64/32-bit (ptr/adr) hybrid mode is compatible with native 64-bit ABI,
# use (replace): RT_ADDRESS=32, rename the binary to RooT.a64_**

# 64-bit packed SIMD mode (fp64/int64) is supported on 64-bit targets,
# use (replace): RT_ELEMENT=64, rename the binary to RooT.a64*64
