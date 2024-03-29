
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


build: RooT_x32

strip:
	strip RooT.x32*

clean:
	rm RooT.x32*


RooT_x32:
	g++ -O3 -g -pthread -mx32 \
        -DRT_LINUX -DRT_X32 -DRT_128=2+4+8 -DRT_256_R8=4 -DRT_256=1+2+8 \
        -DRT_512_R8=1+2 -DRT_512=1+2 -DRT_1K4=1+2 -DRT_SIMD_COMPAT_SSE=2 \
        -DRT_POINTER=32 -DRT_ADDRESS=32 -DRT_ELEMENT=32 -DRT_ENDIAN=0 \
        -DRT_DEBUG=0 -DRT_PATH="../" -DRT_FULLSCREEN=0 \
        -DRT_EMBED_STDOUT=0 -DRT_EMBED_FILEIO=0 -DRT_EMBED_TEX=1 \
        ${INC_PATH} ${SRC_LIST} ${LIB_PATH} ${LIB_LIST} -o RooT.x32


clang: RooT.x32

RooT.x32:
	clang++ -O3 -g -pthread -mx32 \
        -DRT_LINUX -DRT_X32 -DRT_128=2+4+8 -DRT_256_R8=4 -DRT_256=1+2+8 \
        -DRT_512_R8=1+2 -DRT_512=1+2 -DRT_1K4=1+2 -DRT_SIMD_COMPAT_SSE=2 \
        -DRT_POINTER=32 -DRT_ADDRESS=32 -DRT_ELEMENT=32 -DRT_ENDIAN=0 \
        -DRT_DEBUG=0 -DRT_PATH="../" -DRT_FULLSCREEN=0 \
        -DRT_EMBED_STDOUT=0 -DRT_EMBED_FILEIO=0 -DRT_EMBED_TEX=1 \
        ${INC_PATH} ${SRC_LIST} ${LIB_PATH} ${LIB_LIST} -o RooT.x32


# Prerequisites for the build:
# multilib-compiler for x86_64 is installed and in the PATH variable.
# sudo apt-get update
# sudo apt-get install make g++-multilib (plus X11/Xext libs for x32 ABI)
# (installation of g++-multilib removes any g++ cross-compilers)
#
# Compiling/running RooT demo:
# make -f RooT_make_x32.mk
# ./RooT.x32
# (hasn't been verified yet due to lack of available libs, SIMD/CORE tests pass)

# Clang native build should theoretically work too (not tested):
# sudo apt-get update (on Ubuntu add "universe" to "main" /etc/apt/sources.list)
# sudo apt-get install clang g++-multilib (plus X11/Xext libs for x32 ABI)
# (installation of g++-multilib removes any g++ cross-compilers)
# make -f RooT_make_x32.mk clean
# make -f RooT_make_x32.mk clang
# (hasn't been verified yet due to lack of available libs, SIMD/CORE tests pass)

# RooT demo uses runtime SIMD target selection, multiple can be specified above
# on x86 targets top cpuid-value is chosen by default, use -n/-k/-s to override
# 1K4-bit SIMD is achieved by combining pairs of 512-bit registers/instructions
# 2K8-bit SIMD is achieved by combining quads of 512-bit registers/instructions
# For 30-regs 512-bit AVX512F/DQ targets on Skylake-X use (replace): RT_512=4+8

# For interpretation of SIMD build flags check compatibility layer in rtzero.h
# or refer to the corresponding simd_make_***.mk file.

# 32-bit ABI hasn't been fully tested yet due to lack of available libs,
# check out 64/32-bit (ptr/adr) hybrid mode for 64-bit ABI in RooT_make_x64.mk
