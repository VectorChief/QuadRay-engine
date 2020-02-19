
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
        ../core/tracer/tracer_128v2.cpp     \
        ../core/tracer/tracer_128v4.cpp     \
        ../core/tracer/tracer_256v1.cpp     \
        ../core/tracer/tracer_256v2.cpp     \
        ../core/tracer/tracer_256v4.cpp     \
        ../core/tracer/tracer_256v8.cpp     \
        ../core/tracer/tracer_512v1.cpp     \
        ../core/tracer/tracer_512v2.cpp     \
        RooT_linux.cpp

LIB_PATH =

LIB_LIST =                                  \
        -lm                                 \
        -lstdc++                            \
        -lX11                               \
        -lXext                              \
        -lpthread


build: build_p8 build_p9

strip:
	strip RooT.p64*

clean:
	rm RooT.p64*


build_p8: RooT_p64_32Lp8 RooT_p64_64Lp8 RooT_p64f32Lp8 RooT_p64f64Lp8

RooT_p64_32Lp8:
	g++ -O2 -g -pthread -mcpu=power8 \
        -DRT_LINUX -DRT_P64 -DRT_128=1 -DRT_256=1 \
        -DRT_POINTER=64 -DRT_ADDRESS=32 -DRT_ELEMENT=32 -DRT_ENDIAN=0 \
        -DRT_DEBUG=0 -DRT_PATH="../" -DRT_FULLSCREEN=0 \
        -DRT_EMBED_STDOUT=0 -DRT_EMBED_FILEIO=0 -DRT_EMBED_TEX=1 \
        ${INC_PATH} ${SRC_LIST} ${LIB_PATH} ${LIB_LIST} -o RooT.p64_32Lp8

RooT_p64_64Lp8:
	g++ -O2 -g -pthread -mcpu=power8 \
        -DRT_LINUX -DRT_P64 -DRT_128=1 -DRT_256=1 \
        -DRT_POINTER=64 -DRT_ADDRESS=32 -DRT_ELEMENT=64 -DRT_ENDIAN=0 \
        -DRT_DEBUG=0 -DRT_PATH="../" -DRT_FULLSCREEN=0 \
        -DRT_EMBED_STDOUT=0 -DRT_EMBED_FILEIO=0 -DRT_EMBED_TEX=1 \
        ${INC_PATH} ${SRC_LIST} ${LIB_PATH} ${LIB_LIST} -o RooT.p64_64Lp8

RooT_p64f32Lp8:
	g++ -O2 -g -pthread -mcpu=power8 \
        -DRT_LINUX -DRT_P64 -DRT_128=1 -DRT_256=1 \
        -DRT_POINTER=64 -DRT_ADDRESS=64 -DRT_ELEMENT=32 -DRT_ENDIAN=0 \
        -DRT_DEBUG=0 -DRT_PATH="../" -DRT_FULLSCREEN=0 \
        -DRT_EMBED_STDOUT=0 -DRT_EMBED_FILEIO=0 -DRT_EMBED_TEX=1 \
        ${INC_PATH} ${SRC_LIST} ${LIB_PATH} ${LIB_LIST} -o RooT.p64f32Lp8

RooT_p64f64Lp8:
	g++ -O2 -g -pthread -mcpu=power8 \
        -DRT_LINUX -DRT_P64 -DRT_128=1 -DRT_256=1 \
        -DRT_POINTER=64 -DRT_ADDRESS=64 -DRT_ELEMENT=64 -DRT_ENDIAN=0 \
        -DRT_DEBUG=0 -DRT_PATH="../" -DRT_FULLSCREEN=0 \
        -DRT_EMBED_STDOUT=0 -DRT_EMBED_FILEIO=0 -DRT_EMBED_TEX=1 \
        ${INC_PATH} ${SRC_LIST} ${LIB_PATH} ${LIB_LIST} -o RooT.p64f64Lp8


build_p9: RooT_p64_32Lp9 RooT_p64_64Lp9 RooT_p64f32Lp9 RooT_p64f64Lp9

RooT_p64_32Lp9:
	g++ -O2 -g -pthread \
        -DRT_LINUX -DRT_P64 -DRT_128=1+2 -DRT_256=1+2 \
        -DRT_POINTER=64 -DRT_ADDRESS=32 -DRT_ELEMENT=32 -DRT_ENDIAN=0 \
        -DRT_DEBUG=0 -DRT_PATH="../" -DRT_FULLSCREEN=0 \
        -DRT_EMBED_STDOUT=0 -DRT_EMBED_FILEIO=0 -DRT_EMBED_TEX=1 \
        ${INC_PATH} ${SRC_LIST} ${LIB_PATH} ${LIB_LIST} -o RooT.p64_32Lp9

RooT_p64_64Lp9:
	g++ -O2 -g -pthread \
        -DRT_LINUX -DRT_P64 -DRT_128=1+2 -DRT_256=1+2 \
        -DRT_POINTER=64 -DRT_ADDRESS=32 -DRT_ELEMENT=64 -DRT_ENDIAN=0 \
        -DRT_DEBUG=0 -DRT_PATH="../" -DRT_FULLSCREEN=0 \
        -DRT_EMBED_STDOUT=0 -DRT_EMBED_FILEIO=0 -DRT_EMBED_TEX=1 \
        ${INC_PATH} ${SRC_LIST} ${LIB_PATH} ${LIB_LIST} -o RooT.p64_64Lp9

RooT_p64f32Lp9:
	g++ -O2 -g -pthread \
        -DRT_LINUX -DRT_P64 -DRT_128=1+2 -DRT_256=1+2 \
        -DRT_POINTER=64 -DRT_ADDRESS=64 -DRT_ELEMENT=32 -DRT_ENDIAN=0 \
        -DRT_DEBUG=0 -DRT_PATH="../" -DRT_FULLSCREEN=0 \
        -DRT_EMBED_STDOUT=0 -DRT_EMBED_FILEIO=0 -DRT_EMBED_TEX=1 \
        ${INC_PATH} ${SRC_LIST} ${LIB_PATH} ${LIB_LIST} -o RooT.p64f32Lp9

RooT_p64f64Lp9:
	g++ -O2 -g -pthread \
        -DRT_LINUX -DRT_P64 -DRT_128=1+2 -DRT_256=1+2 \
        -DRT_POINTER=64 -DRT_ADDRESS=64 -DRT_ELEMENT=64 -DRT_ENDIAN=0 \
        -DRT_DEBUG=0 -DRT_PATH="../" -DRT_FULLSCREEN=0 \
        -DRT_EMBED_STDOUT=0 -DRT_EMBED_FILEIO=0 -DRT_EMBED_TEX=1 \
        ${INC_PATH} ${SRC_LIST} ${LIB_PATH} ${LIB_LIST} -o RooT.p64f64Lp9


clang: clang_p8 clang_p9

clang_p8: RooT.p64_32Lp8 RooT.p64_64Lp8 RooT.p64f32Lp8 RooT.p64f64Lp8

RooT.p64_32Lp8:
	clang++ -O3 -g -pthread -mcpu=power8 \
        -DRT_LINUX -DRT_P64 -DRT_128=1 -DRT_256=1 \
        -DRT_POINTER=64 -DRT_ADDRESS=32 -DRT_ELEMENT=32 -DRT_ENDIAN=0 \
        -DRT_DEBUG=0 -DRT_PATH="../" -DRT_FULLSCREEN=0 \
        -DRT_EMBED_STDOUT=0 -DRT_EMBED_FILEIO=0 -DRT_EMBED_TEX=1 \
        ${INC_PATH} ${SRC_LIST} ${LIB_PATH} ${LIB_LIST} -o RooT.p64_32Lp8

RooT.p64_64Lp8:
	clang++ -O3 -g -pthread -mcpu=power8 \
        -DRT_LINUX -DRT_P64 -DRT_128=1 -DRT_256=1 \
        -DRT_POINTER=64 -DRT_ADDRESS=32 -DRT_ELEMENT=64 -DRT_ENDIAN=0 \
        -DRT_DEBUG=0 -DRT_PATH="../" -DRT_FULLSCREEN=0 \
        -DRT_EMBED_STDOUT=0 -DRT_EMBED_FILEIO=0 -DRT_EMBED_TEX=1 \
        ${INC_PATH} ${SRC_LIST} ${LIB_PATH} ${LIB_LIST} -o RooT.p64_64Lp8

RooT.p64f32Lp8:
	clang++ -O3 -g -pthread -mcpu=power8 \
        -DRT_LINUX -DRT_P64 -DRT_128=1 -DRT_256=1 \
        -DRT_POINTER=64 -DRT_ADDRESS=64 -DRT_ELEMENT=32 -DRT_ENDIAN=0 \
        -DRT_DEBUG=0 -DRT_PATH="../" -DRT_FULLSCREEN=0 \
        -DRT_EMBED_STDOUT=0 -DRT_EMBED_FILEIO=0 -DRT_EMBED_TEX=1 \
        ${INC_PATH} ${SRC_LIST} ${LIB_PATH} ${LIB_LIST} -o RooT.p64f32Lp8

RooT.p64f64Lp8:
	clang++ -O3 -g -pthread -mcpu=power8 \
        -DRT_LINUX -DRT_P64 -DRT_128=1 -DRT_256=1 \
        -DRT_POINTER=64 -DRT_ADDRESS=64 -DRT_ELEMENT=64 -DRT_ENDIAN=0 \
        -DRT_DEBUG=0 -DRT_PATH="../" -DRT_FULLSCREEN=0 \
        -DRT_EMBED_STDOUT=0 -DRT_EMBED_FILEIO=0 -DRT_EMBED_TEX=1 \
        ${INC_PATH} ${SRC_LIST} ${LIB_PATH} ${LIB_LIST} -o RooT.p64f64Lp8


clang_p9: RooT.p64_32Lp9 RooT.p64_64Lp9 RooT.p64f32Lp9 RooT.p64f64Lp9

RooT.p64_32Lp9:
	clang++ -O3 -g -pthread \
        -DRT_LINUX -DRT_P64 -DRT_128=1+2 -DRT_256=1+2 \
        -DRT_POINTER=64 -DRT_ADDRESS=32 -DRT_ELEMENT=32 -DRT_ENDIAN=0 \
        -DRT_DEBUG=0 -DRT_PATH="../" -DRT_FULLSCREEN=0 \
        -DRT_EMBED_STDOUT=0 -DRT_EMBED_FILEIO=0 -DRT_EMBED_TEX=1 \
        ${INC_PATH} ${SRC_LIST} ${LIB_PATH} ${LIB_LIST} -o RooT.p64_32Lp9

RooT.p64_64Lp9:
	clang++ -O3 -g -pthread \
        -DRT_LINUX -DRT_P64 -DRT_128=1+2 -DRT_256=1+2 \
        -DRT_POINTER=64 -DRT_ADDRESS=32 -DRT_ELEMENT=64 -DRT_ENDIAN=0 \
        -DRT_DEBUG=0 -DRT_PATH="../" -DRT_FULLSCREEN=0 \
        -DRT_EMBED_STDOUT=0 -DRT_EMBED_FILEIO=0 -DRT_EMBED_TEX=1 \
        ${INC_PATH} ${SRC_LIST} ${LIB_PATH} ${LIB_LIST} -o RooT.p64_64Lp9

RooT.p64f32Lp9:
	clang++ -O3 -g -pthread \
        -DRT_LINUX -DRT_P64 -DRT_128=1+2 -DRT_256=1+2 \
        -DRT_POINTER=64 -DRT_ADDRESS=64 -DRT_ELEMENT=32 -DRT_ENDIAN=0 \
        -DRT_DEBUG=0 -DRT_PATH="../" -DRT_FULLSCREEN=0 \
        -DRT_EMBED_STDOUT=0 -DRT_EMBED_FILEIO=0 -DRT_EMBED_TEX=1 \
        ${INC_PATH} ${SRC_LIST} ${LIB_PATH} ${LIB_LIST} -o RooT.p64f32Lp9

RooT.p64f64Lp9:
	clang++ -O3 -g -pthread \
        -DRT_LINUX -DRT_P64 -DRT_128=1+2 -DRT_256=1+2 \
        -DRT_POINTER=64 -DRT_ADDRESS=64 -DRT_ELEMENT=64 -DRT_ENDIAN=0 \
        -DRT_DEBUG=0 -DRT_PATH="../" -DRT_FULLSCREEN=0 \
        -DRT_EMBED_STDOUT=0 -DRT_EMBED_FILEIO=0 -DRT_EMBED_TEX=1 \
        ${INC_PATH} ${SRC_LIST} ${LIB_PATH} ${LIB_LIST} -o RooT.p64f64Lp9


# Prerequisites for the build:
# native-compiler for 64-bit POWER is installed and in the PATH variable.
# sudo apt-get update (on POWER host or QEMU system mode)
# sudo apt-get install make g++ libxext-dev
# (recent g++-5-powerpc64le series target POWER8 and don't work well with -O3)
#
# Compiling/running RooT demo:
# make -f RooT_make_p64.mk
# ./RooT.p64f32Lp8 (on POWER8 host or QEMU system mode)
# ./RooT.p64f32Lp9 (on POWER9 host or QEMU system mode)
# (has been tested on Tyan TN71-BP012 POWER8 with Ubuntu Server 16.04/18.04)
# (SIMD and CORE tests pass in QEMU linux-user mode, check test subfolder)

# Clang native build works too (takes much longer prior to 3.8):
# sudo apt-get update (on Ubuntu add "universe" to "main" /etc/apt/sources.list)
# sudo apt-get install clang libxext-dev (on POWER host or QEMU system mode)
# make -f RooT_make_p64.mk clean
# make -f RooT_make_p64.mk clang -j4

# RooT demo uses runtime SIMD target selection, multiple can be specified above
# on RISC targets top value above is chosen by default, use -n/-k/-s to override
# 256-bit SIMD is achieved by combining pairs of 128-bit registers/instructions
# 512-bit SIMD is achieved by combining quads of 128-bit registers/instructions
# For 30 256-bit VSX2/3 registers on POWER8/9 targets use (replace): RT_256=4+8
# For 15 512-bit VSX2/3 registers on POWER8/9 targets use (replace): RT_512=1+2

# For interpretation of SIMD build flags check compatibility layer in rtzero.h
# or refer to the corresponding simd_make_***.mk file.

# 64/32-bit (ptr/adr) hybrid mode is compatible with native 64-bit ABI,
# use (replace): RT_ADDRESS=32, rename the binary to RooT.p64_**
# 64-bit packed SIMD mode (fp64/int64) is supported on 64-bit targets,
# use (replace): RT_ELEMENT=64, rename the binary to RooT.p64*64
