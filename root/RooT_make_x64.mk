
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
        ../core/tracer/tracer_128v2.cpp     \
        ../core/tracer/tracer_128v4.cpp     \
        ../core/tracer/tracer_128v8.cpp     \
        ../core/tracer/tracer_256v4_r8.cpp  \
        ../core/tracer/tracer_256v1.cpp     \
        ../core/tracer/tracer_256v2.cpp     \
        ../core/tracer/tracer_256v8.cpp     \
        ../core/tracer/tracer_512v1_r8.cpp  \
        ../core/tracer/tracer_512v2_r8.cpp  \
        ../core/tracer/tracer_512v1.cpp     \
        ../core/tracer/tracer_512v2.cpp     \
        ../core/tracer/tracer_512v4.cpp     \
        ../core/tracer/tracer_512v8.cpp     \
        ../core/tracer/tracer_1K4v1.cpp     \
        ../core/tracer/tracer_1K4v2.cpp     \
        ../core/tracer/tracer_2K8v1_r8.cpp  \
        ../core/tracer/tracer_2K8v2_r8.cpp  \
        RooT_linux.cpp

LIB_PATH =

LIB_LIST =                                  \
        -lm                                 \
        -lstdc++                            \
        -lX11                               \
        -lXext                              \
        -lpthread


build: RooT_x64_32 RooT_x64_64 RooT_x64f32 RooT_x64f64

strip:
	strip RooT.x64*

clean:
	rm RooT.x64*


RooT_x64_32:
	g++ -O3 -g -pthread \
        -DRT_LINUX -DRT_X64 -DRT_128=2+4+8 -DRT_256_R8=4 -DRT_256=1+2+8 \
        -DRT_512_R8=1+2 -DRT_512=1+2 -DRT_1K4=1+2 -DRT_2K8_R8=0 \
        -DRT_POINTER=64 -DRT_ADDRESS=32 -DRT_ELEMENT=32 -DRT_ENDIAN=0 \
        -DRT_DEBUG=0 -DRT_PATH="../" -DRT_FULLSCREEN=0 \
        -DRT_EMBED_STDOUT=0 -DRT_EMBED_FILEIO=0 -DRT_EMBED_TEX=1 \
        ${INC_PATH} ${SRC_LIST} ${LIB_PATH} ${LIB_LIST} -o RooT.x64_32

RooT_x64_64:
	g++ -O3 -g -pthread \
        -DRT_LINUX -DRT_X64 -DRT_128=2+4+8 -DRT_256_R8=4 -DRT_256=1+2+8 \
        -DRT_512_R8=1+2 -DRT_512=1+2 -DRT_1K4=1+2 -DRT_2K8_R8=0 \
        -DRT_POINTER=64 -DRT_ADDRESS=32 -DRT_ELEMENT=64 -DRT_ENDIAN=0 \
        -DRT_DEBUG=0 -DRT_PATH="../" -DRT_FULLSCREEN=0 \
        -DRT_EMBED_STDOUT=0 -DRT_EMBED_FILEIO=0 -DRT_EMBED_TEX=1 \
        ${INC_PATH} ${SRC_LIST} ${LIB_PATH} ${LIB_LIST} -o RooT.x64_64

RooT_x64f32:
	g++ -O3 -g -pthread \
        -DRT_LINUX -DRT_X64 -DRT_128=2+4+8 -DRT_256_R8=4 -DRT_256=1+2+8 \
        -DRT_512_R8=1+2 -DRT_512=1+2 -DRT_1K4=1+2 -DRT_2K8_R8=0 \
        -DRT_POINTER=64 -DRT_ADDRESS=64 -DRT_ELEMENT=32 -DRT_ENDIAN=0 \
        -DRT_DEBUG=0 -DRT_PATH="../" -DRT_FULLSCREEN=0 \
        -DRT_EMBED_STDOUT=0 -DRT_EMBED_FILEIO=0 -DRT_EMBED_TEX=1 \
        ${INC_PATH} ${SRC_LIST} ${LIB_PATH} ${LIB_LIST} -o RooT.x64f32

RooT_x64f64:
	g++ -O3 -g -pthread \
        -DRT_LINUX -DRT_X64 -DRT_128=2+4+8 -DRT_256_R8=4 -DRT_256=1+2+8 \
        -DRT_512_R8=1+2 -DRT_512=1+2 -DRT_1K4=1+2 -DRT_2K8_R8=0 \
        -DRT_POINTER=64 -DRT_ADDRESS=64 -DRT_ELEMENT=64 -DRT_ENDIAN=0 \
        -DRT_DEBUG=0 -DRT_PATH="../" -DRT_FULLSCREEN=0 \
        -DRT_EMBED_STDOUT=0 -DRT_EMBED_FILEIO=0 -DRT_EMBED_TEX=1 \
        ${INC_PATH} ${SRC_LIST} ${LIB_PATH} ${LIB_LIST} -o RooT.x64f64


clang: RooT.x64_32 RooT.x64_64 RooT.x64f32 RooT.x64f64

RooT.x64_32:
	clang++ -O3 -g -pthread \
        -Wno-unknown-warning-option \
        -Wno-shift-negative-value -Wno-shift-op-parentheses \
        -Wno-logical-op-parentheses -Wno-bitwise-op-parentheses \
        -DRT_LINUX -DRT_X64 -DRT_128=2+4+8 -DRT_256_R8=4 -DRT_256=1+2+8 \
        -DRT_512_R8=1+2 -DRT_512=1+2 -DRT_1K4=1+2 -DRT_2K8_R8=0 \
        -DRT_POINTER=64 -DRT_ADDRESS=32 -DRT_ELEMENT=32 -DRT_ENDIAN=0 \
        -DRT_DEBUG=0 -DRT_PATH="../" -DRT_FULLSCREEN=0 \
        -DRT_EMBED_STDOUT=0 -DRT_EMBED_FILEIO=0 -DRT_EMBED_TEX=1 \
        ${INC_PATH} ${SRC_LIST} ${LIB_PATH} ${LIB_LIST} -o RooT.x64_32

RooT.x64_64:
	clang++ -O3 -g -pthread \
        -Wno-unknown-warning-option \
        -Wno-shift-negative-value -Wno-shift-op-parentheses \
        -Wno-logical-op-parentheses -Wno-bitwise-op-parentheses \
        -DRT_LINUX -DRT_X64 -DRT_128=2+4+8 -DRT_256_R8=4 -DRT_256=1+2+8 \
        -DRT_512_R8=1+2 -DRT_512=1+2 -DRT_1K4=1+2 -DRT_2K8_R8=0 \
        -DRT_POINTER=64 -DRT_ADDRESS=32 -DRT_ELEMENT=64 -DRT_ENDIAN=0 \
        -DRT_DEBUG=0 -DRT_PATH="../" -DRT_FULLSCREEN=0 \
        -DRT_EMBED_STDOUT=0 -DRT_EMBED_FILEIO=0 -DRT_EMBED_TEX=1 \
        ${INC_PATH} ${SRC_LIST} ${LIB_PATH} ${LIB_LIST} -o RooT.x64_64

RooT.x64f32:
	clang++ -O3 -g -pthread \
        -Wno-unknown-warning-option \
        -Wno-shift-negative-value -Wno-shift-op-parentheses \
        -Wno-logical-op-parentheses -Wno-bitwise-op-parentheses \
        -DRT_LINUX -DRT_X64 -DRT_128=2+4+8 -DRT_256_R8=4 -DRT_256=1+2+8 \
        -DRT_512_R8=1+2 -DRT_512=1+2 -DRT_1K4=1+2 -DRT_2K8_R8=0 \
        -DRT_POINTER=64 -DRT_ADDRESS=64 -DRT_ELEMENT=32 -DRT_ENDIAN=0 \
        -DRT_DEBUG=0 -DRT_PATH="../" -DRT_FULLSCREEN=0 \
        -DRT_EMBED_STDOUT=0 -DRT_EMBED_FILEIO=0 -DRT_EMBED_TEX=1 \
        ${INC_PATH} ${SRC_LIST} ${LIB_PATH} ${LIB_LIST} -o RooT.x64f32

RooT.x64f64:
	clang++ -O3 -g -pthread \
        -Wno-unknown-warning-option \
        -Wno-shift-negative-value -Wno-shift-op-parentheses \
        -Wno-logical-op-parentheses -Wno-bitwise-op-parentheses \
        -DRT_LINUX -DRT_X64 -DRT_128=2+4+8 -DRT_256_R8=4 -DRT_256=1+2+8 \
        -DRT_512_R8=1+2 -DRT_512=1+2 -DRT_1K4=1+2 -DRT_2K8_R8=0 \
        -DRT_POINTER=64 -DRT_ADDRESS=64 -DRT_ELEMENT=64 -DRT_ENDIAN=0 \
        -DRT_DEBUG=0 -DRT_PATH="../" -DRT_FULLSCREEN=0 \
        -DRT_EMBED_STDOUT=0 -DRT_EMBED_FILEIO=0 -DRT_EMBED_TEX=1 \
        ${INC_PATH} ${SRC_LIST} ${LIB_PATH} ${LIB_LIST} -o RooT.x64f64


# Prerequisites for the build:
# native-compiler for x86_64 is installed and in the PATH variable.
# sudo apt-get update
# sudo apt-get install g++ libxext-dev (on x86_64 host)
#
# Building/running RooT demo:
# make -f RooT_make_x64.mk
# ./RooT.x64f32

# clang compilation takes much longer prior to 3.8 (older Ubuntu 14.04/Mint 17)
# sudo apt-get update (on Ubuntu add "universe" to "main" /etc/apt/sources.list)
# sudo apt-get install clang libxext-dev (on x86_64 host)
# make -f RooT_make_x64.mk clang

# RooT demo uses runtime SIMD target selection, multiple can be specified above
# on x86 targets top cpuid-value is chosen by default, use -n/-k/-s to override
# 1K4-bit SIMD is achieved by combining pairs of 512-bit registers/instructions
# 2K8-bit SIMD is achieved by combining quads of 512-bit registers/instructions
# For 30-regs 512-bit AVX512F/DQ targets on Skylake-X use (replace): RT_512=4+8

# For interpretation of SIMD build flags check compatibility layer in rtzero.h

# 64/32-bit (ptr/adr) hybrid mode is compatible with native 64-bit ABI,
# use (replace): RT_ADDRESS=32, rename the binary to RooT.x64_**

# 64-bit packed SIMD mode (fp64/int64) is supported on 64-bit targets,
# use (replace): RT_ELEMENT=64, rename the binary to RooT.x64*64
