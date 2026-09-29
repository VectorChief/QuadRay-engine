
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
        -lX11                               \
        -lXext                              \
        -lpthread


build: RooT_z64_32 RooT_z64_64 RooT_z64f32 RooT_z64f64

strip:
	strip RooT.z64*

clean:
	rm RooT.z64*


RooT_z64_32:
	g++ -O3 -g -pthread \
        -DRT_LINUX -DRT_Z64 -DRT_128=1 -DRT_256=1 \
        -DRT_POINTER=64 -DRT_ADDRESS=32 -DRT_ELEMENT=32 -DRT_ENDIAN=1 \
        -DRT_DEBUG=0 -DRT_PATH="../" -DRT_FULLSCREEN=0 \
        -DRT_EMBED_STDOUT=0 -DRT_EMBED_FILEIO=0 -DRT_EMBED_TEX=1 \
        ${INC_PATH} ${SRC_LIST} ${LIB_PATH} ${LIB_LIST} -o RooT.z64_32

RooT_z64_64:
	g++ -O3 -g -pthread \
        -DRT_LINUX -DRT_Z64 -DRT_128=1 -DRT_256=1 \
        -DRT_POINTER=64 -DRT_ADDRESS=32 -DRT_ELEMENT=64 -DRT_ENDIAN=1 \
        -DRT_DEBUG=0 -DRT_PATH="../" -DRT_FULLSCREEN=0 \
        -DRT_EMBED_STDOUT=0 -DRT_EMBED_FILEIO=0 -DRT_EMBED_TEX=1 \
        ${INC_PATH} ${SRC_LIST} ${LIB_PATH} ${LIB_LIST} -o RooT.z64_64

RooT_z64f32:
	g++ -O3 -g -pthread \
        -DRT_LINUX -DRT_Z64 -DRT_128=1 -DRT_256=1 \
        -DRT_POINTER=64 -DRT_ADDRESS=64 -DRT_ELEMENT=32 -DRT_ENDIAN=1 \
        -DRT_DEBUG=0 -DRT_PATH="../" -DRT_FULLSCREEN=0 \
        -DRT_EMBED_STDOUT=0 -DRT_EMBED_FILEIO=0 -DRT_EMBED_TEX=1 \
        ${INC_PATH} ${SRC_LIST} ${LIB_PATH} ${LIB_LIST} -o RooT.z64f32

RooT_z64f64:
	g++ -O3 -g -pthread \
        -DRT_LINUX -DRT_Z64 -DRT_128=1 -DRT_256=1 \
        -DRT_POINTER=64 -DRT_ADDRESS=64 -DRT_ELEMENT=64 -DRT_ENDIAN=1 \
        -DRT_DEBUG=0 -DRT_PATH="../" -DRT_FULLSCREEN=0 \
        -DRT_EMBED_STDOUT=0 -DRT_EMBED_FILEIO=0 -DRT_EMBED_TEX=1 \
        ${INC_PATH} ${SRC_LIST} ${LIB_PATH} ${LIB_LIST} -o RooT.z64f64


clang: RooT.z64_32 RooT.z64_64 RooT.z64f32 RooT.z64f64

RooT.z64_32:
	clang++ -O0 -g -pthread \
        -DRT_LINUX -DRT_P64 -DRT_128=1 -DRT_256=1 \
        -DRT_POINTER=64 -DRT_ADDRESS=32 -DRT_ELEMENT=32 -DRT_ENDIAN=1 \
        -DRT_DEBUG=0 -DRT_PATH="../" -DRT_FULLSCREEN=0 \
        -DRT_EMBED_STDOUT=0 -DRT_EMBED_FILEIO=0 -DRT_EMBED_TEX=1 \
        ${INC_PATH} ${SRC_LIST} ${LIB_PATH} ${LIB_LIST} -o RooT.z64_32

RooT.z64_64:
	clang++ -O0 -g -pthread \
        -DRT_LINUX -DRT_P64 -DRT_128=1 -DRT_256=1 \
        -DRT_POINTER=64 -DRT_ADDRESS=32 -DRT_ELEMENT=64 -DRT_ENDIAN=1 \
        -DRT_DEBUG=0 -DRT_PATH="../" -DRT_FULLSCREEN=0 \
        -DRT_EMBED_STDOUT=0 -DRT_EMBED_FILEIO=0 -DRT_EMBED_TEX=1 \
        ${INC_PATH} ${SRC_LIST} ${LIB_PATH} ${LIB_LIST} -o RooT.z64_64

RooT.z64f32:
	clang++ -O0 -g -pthread \
        -DRT_LINUX -DRT_P64 -DRT_128=1 -DRT_256=1 \
        -DRT_POINTER=64 -DRT_ADDRESS=64 -DRT_ELEMENT=32 -DRT_ENDIAN=1 \
        -DRT_DEBUG=0 -DRT_PATH="../" -DRT_FULLSCREEN=0 \
        -DRT_EMBED_STDOUT=0 -DRT_EMBED_FILEIO=0 -DRT_EMBED_TEX=1 \
        ${INC_PATH} ${SRC_LIST} ${LIB_PATH} ${LIB_LIST} -o RooT.z64f32

RooT.z64f64:
	clang++ -O0 -g -pthread \
        -DRT_LINUX -DRT_P64 -DRT_128=1 -DRT_256=1 \
        -DRT_POINTER=64 -DRT_ADDRESS=64 -DRT_ELEMENT=64 -DRT_ENDIAN=1 \
        -DRT_DEBUG=0 -DRT_PATH="../" -DRT_FULLSCREEN=0 \
        -DRT_EMBED_STDOUT=0 -DRT_EMBED_FILEIO=0 -DRT_EMBED_TEX=1 \
        ${INC_PATH} ${SRC_LIST} ${LIB_PATH} ${LIB_LIST} -o RooT.z64f64


# Prerequisites for the build:
# native-compiler for 64-bit Z Systems is installed and in the PATH variable.
# sudo apt-get update (on Z15/17 host or QEMU system mode)
# sudo apt-get install make g++ libxext-dev
#
# Compiling/running RooT demo:
# make -f RooT_make_z64.mk
# ./RooT.z64f32 (on Z15/17 host or QEMU system mode)

# Clang native build should theoretically work too (not tested):
# sudo apt-get update (on Ubuntu add "universe" to "main" /etc/apt/sources.list)
# sudo apt-get install clang libxext-dev (on Z15/17 host or QEMU system mode)
# make -f RooT_make_z64.mk clean
# make -f RooT_make_z64.mk clang -j4

# RooT demo uses runtime SIMD target selection, multiple can be specified above
# on RISC targets top value above is chosen by default, use -n/-k/-s to override
# 256-bit SIMD is achieved by combining pairs of 128-bit registers/instructions

# For interpretation of SIMD build flags check compatibility layer in rtzero.h
# or refer to the corresponding simd_make_***.mk file.

# 64/32-bit (ptr/adr) hybrid mode is compatible with native 64-bit ABI,
# use (replace): RT_ADDRESS=32, rename the binary to RooT.z64_**
# 64-bit packed SIMD mode (fp64/int64) is supported on 64-bit targets,
# use (replace): RT_ELEMENT=64, rename the binary to RooT.z64*64
