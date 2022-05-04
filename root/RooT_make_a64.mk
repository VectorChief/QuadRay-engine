
INC_PATH =                                  \
        -I../core/config/                   \
        -I../core/engine/                   \
        -I../core/system/                   \
        -I../core/tracer/                   \
        -I../data/materials/                \
        -I../data/objects/                  \
        -I../data/scenes/                   \
        -I../data/textures/                 \
        -I/usr/X11/include/

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
        ../core/tracer/tracer_256v4.cpp     \
        ../core/tracer/tracer_512v1.cpp     \
        ../core/tracer/tracer_512v4.cpp     \
        ../core/tracer/tracer_1K4v1.cpp     \
        ../core/tracer/tracer_1K4v4.cpp     \
        ../core/tracer/tracer_2K8v1_r8.cpp  \
        ../core/tracer/tracer_2K8v4_r8.cpp  \
        RooT_linux.cpp

LIB_PATH =                                  \
        -L/usr/X11/lib/

LIB_LIST =                                  \
        -lm                                 \
        -lstdc++                            \
        -lX11                               \
        -lXext                              \
        -lpthread


build: build_a64 build_a64sve
clang: clang_a64 clang_a64sve

strip:
	strip RooT.a64*

clean:
	rm RooT.a64*

macOS:
	mv RooT.a64_32 RooT.d64_32
	mv RooT.a64_64 RooT.d64_64
	mv RooT.a64f32 RooT.d64f32
	mv RooT.a64f64 RooT.d64f64
	mv RooT.a64_32sve RooT.d64_32sve
	mv RooT.a64_64sve RooT.d64_64sve
	mv RooT.a64f32sve RooT.d64f32sve
	mv RooT.a64f64sve RooT.d64f64sve

macRD:
	rm -fr RooT.a64*.dSYM/

macRM:
	rm RooT.d64*


build_a64: RooT_a64_32 RooT_a64_64 RooT_a64f32 RooT_a64f64

RooT_a64_32:
	g++ -O3 -g -pthread \
        -DRT_LINUX -DRT_A64 -DRT_128=1 -DRT_256=1 \
        -DRT_POINTER=64 -DRT_ADDRESS=32 -DRT_ELEMENT=32 -DRT_ENDIAN=0 \
        -DRT_DEBUG=0 -DRT_PATH="../" -DRT_FULLSCREEN=0 \
        -DRT_EMBED_STDOUT=0 -DRT_EMBED_FILEIO=0 -DRT_EMBED_TEX=1 \
        ${INC_PATH} ${SRC_LIST} ${LIB_PATH} ${LIB_LIST} -o RooT.a64_32

RooT_a64_64:
	g++ -O3 -g -pthread \
        -DRT_LINUX -DRT_A64 -DRT_128=1 -DRT_256=1 \
        -DRT_POINTER=64 -DRT_ADDRESS=32 -DRT_ELEMENT=64 -DRT_ENDIAN=0 \
        -DRT_DEBUG=0 -DRT_PATH="../" -DRT_FULLSCREEN=0 \
        -DRT_EMBED_STDOUT=0 -DRT_EMBED_FILEIO=0 -DRT_EMBED_TEX=1 \
        ${INC_PATH} ${SRC_LIST} ${LIB_PATH} ${LIB_LIST} -o RooT.a64_64

RooT_a64f32:
	g++ -O3 -g -pthread \
        -DRT_LINUX -DRT_A64 -DRT_128=1 -DRT_256=1 \
        -DRT_POINTER=64 -DRT_ADDRESS=64 -DRT_ELEMENT=32 -DRT_ENDIAN=0 \
        -DRT_DEBUG=0 -DRT_PATH="../" -DRT_FULLSCREEN=0 \
        -DRT_EMBED_STDOUT=0 -DRT_EMBED_FILEIO=0 -DRT_EMBED_TEX=1 \
        ${INC_PATH} ${SRC_LIST} ${LIB_PATH} ${LIB_LIST} -o RooT.a64f32

RooT_a64f64:
	g++ -O3 -g -pthread \
        -DRT_LINUX -DRT_A64 -DRT_128=1 -DRT_256=1 \
        -DRT_POINTER=64 -DRT_ADDRESS=64 -DRT_ELEMENT=64 -DRT_ENDIAN=0 \
        -DRT_DEBUG=0 -DRT_PATH="../" -DRT_FULLSCREEN=0 \
        -DRT_EMBED_STDOUT=0 -DRT_EMBED_FILEIO=0 -DRT_EMBED_TEX=1 \
        ${INC_PATH} ${SRC_LIST} ${LIB_PATH} ${LIB_LIST} -o RooT.a64f64


build_a64sve: RooT_a64_32sve RooT_a64_64sve \
              RooT_a64f32sve RooT_a64f64sve

RooT_a64_32sve:
	g++ -O3 -g -pthread \
        -DRT_LINUX -DRT_A64 -DRT_128=1 -DRT_256=1+4 -DRT_512=1+4 -DRT_1K4=1+4 \
        -DRT_POINTER=64 -DRT_ADDRESS=32 -DRT_ELEMENT=32 -DRT_ENDIAN=0 \
        -DRT_DEBUG=0 -DRT_PATH="../" -DRT_FULLSCREEN=0 \
        -DRT_EMBED_STDOUT=0 -DRT_EMBED_FILEIO=0 -DRT_EMBED_TEX=1 \
        ${INC_PATH} ${SRC_LIST} ${LIB_PATH} ${LIB_LIST} -o RooT.a64_32sve

RooT_a64_64sve:
	g++ -O3 -g -pthread \
        -DRT_LINUX -DRT_A64 -DRT_128=1 -DRT_256=1+4 -DRT_512=1+4 -DRT_1K4=1+4 \
        -DRT_POINTER=64 -DRT_ADDRESS=32 -DRT_ELEMENT=64 -DRT_ENDIAN=0 \
        -DRT_DEBUG=0 -DRT_PATH="../" -DRT_FULLSCREEN=0 \
        -DRT_EMBED_STDOUT=0 -DRT_EMBED_FILEIO=0 -DRT_EMBED_TEX=1 \
        ${INC_PATH} ${SRC_LIST} ${LIB_PATH} ${LIB_LIST} -o RooT.a64_64sve

RooT_a64f32sve:
	g++ -O3 -g -pthread \
        -DRT_LINUX -DRT_A64 -DRT_128=1 -DRT_256=1+4 -DRT_512=1+4 -DRT_1K4=1+4 \
        -DRT_POINTER=64 -DRT_ADDRESS=64 -DRT_ELEMENT=32 -DRT_ENDIAN=0 \
        -DRT_DEBUG=0 -DRT_PATH="../" -DRT_FULLSCREEN=0 \
        -DRT_EMBED_STDOUT=0 -DRT_EMBED_FILEIO=0 -DRT_EMBED_TEX=1 \
        ${INC_PATH} ${SRC_LIST} ${LIB_PATH} ${LIB_LIST} -o RooT.a64f32sve

RooT_a64f64sve:
	g++ -O3 -g -pthread \
        -DRT_LINUX -DRT_A64 -DRT_128=1 -DRT_256=1+4 -DRT_512=1+4 -DRT_1K4=1+4 \
        -DRT_POINTER=64 -DRT_ADDRESS=64 -DRT_ELEMENT=64 -DRT_ENDIAN=0 \
        -DRT_DEBUG=0 -DRT_PATH="../" -DRT_FULLSCREEN=0 \
        -DRT_EMBED_STDOUT=0 -DRT_EMBED_FILEIO=0 -DRT_EMBED_TEX=1 \
        ${INC_PATH} ${SRC_LIST} ${LIB_PATH} ${LIB_LIST} -o RooT.a64f64sve


clang_a64: RooT.a64_32 RooT.a64_64 RooT.a64f32 RooT.a64f64

RooT.a64_32:
	clang++ -O3 -g -pthread \
        -DRT_LINUX -DRT_A64 -DRT_128=1 -DRT_256=1 \
        -DRT_POINTER=64 -DRT_ADDRESS=32 -DRT_ELEMENT=32 -DRT_ENDIAN=0 \
        -DRT_DEBUG=0 -DRT_PATH="../" -DRT_FULLSCREEN=0 \
        -DRT_EMBED_STDOUT=0 -DRT_EMBED_FILEIO=0 -DRT_EMBED_TEX=1 \
        ${INC_PATH} ${SRC_LIST} ${LIB_PATH} ${LIB_LIST} -o RooT.a64_32

RooT.a64_64:
	clang++ -O3 -g -pthread \
        -DRT_LINUX -DRT_A64 -DRT_128=1 -DRT_256=1 \
        -DRT_POINTER=64 -DRT_ADDRESS=32 -DRT_ELEMENT=64 -DRT_ENDIAN=0 \
        -DRT_DEBUG=0 -DRT_PATH="../" -DRT_FULLSCREEN=0 \
        -DRT_EMBED_STDOUT=0 -DRT_EMBED_FILEIO=0 -DRT_EMBED_TEX=1 \
        ${INC_PATH} ${SRC_LIST} ${LIB_PATH} ${LIB_LIST} -o RooT.a64_64

RooT.a64f32:
	clang++ -O3 -g -pthread \
        -DRT_LINUX -DRT_A64 -DRT_128=1 -DRT_256=1 \
        -DRT_POINTER=64 -DRT_ADDRESS=64 -DRT_ELEMENT=32 -DRT_ENDIAN=0 \
        -DRT_DEBUG=0 -DRT_PATH="../" -DRT_FULLSCREEN=0 \
        -DRT_EMBED_STDOUT=0 -DRT_EMBED_FILEIO=0 -DRT_EMBED_TEX=1 \
        ${INC_PATH} ${SRC_LIST} ${LIB_PATH} ${LIB_LIST} -o RooT.a64f32

RooT.a64f64:
	clang++ -O3 -g -pthread \
        -DRT_LINUX -DRT_A64 -DRT_128=1 -DRT_256=1 \
        -DRT_POINTER=64 -DRT_ADDRESS=64 -DRT_ELEMENT=64 -DRT_ENDIAN=0 \
        -DRT_DEBUG=0 -DRT_PATH="../" -DRT_FULLSCREEN=0 \
        -DRT_EMBED_STDOUT=0 -DRT_EMBED_FILEIO=0 -DRT_EMBED_TEX=1 \
        ${INC_PATH} ${SRC_LIST} ${LIB_PATH} ${LIB_LIST} -o RooT.a64f64


clang_a64sve: RooT.a64_32sve RooT.a64_64sve \
              RooT.a64f32sve RooT.a64f64sve

RooT.a64_32sve:
	clang++ -O3 -g -pthread \
        -DRT_LINUX -DRT_A64 -DRT_128=1 -DRT_256=1+4 -DRT_512=1+4 -DRT_1K4=1+4 \
        -DRT_POINTER=64 -DRT_ADDRESS=32 -DRT_ELEMENT=32 -DRT_ENDIAN=0 \
        -DRT_DEBUG=0 -DRT_PATH="../" -DRT_FULLSCREEN=0 \
        -DRT_EMBED_STDOUT=0 -DRT_EMBED_FILEIO=0 -DRT_EMBED_TEX=1 \
        ${INC_PATH} ${SRC_LIST} ${LIB_PATH} ${LIB_LIST} -o RooT.a64_32sve

RooT.a64_64sve:
	clang++ -O3 -g -pthread \
        -DRT_LINUX -DRT_A64 -DRT_128=1 -DRT_256=1+4 -DRT_512=1+4 -DRT_1K4=1+4 \
        -DRT_POINTER=64 -DRT_ADDRESS=32 -DRT_ELEMENT=64 -DRT_ENDIAN=0 \
        -DRT_DEBUG=0 -DRT_PATH="../" -DRT_FULLSCREEN=0 \
        -DRT_EMBED_STDOUT=0 -DRT_EMBED_FILEIO=0 -DRT_EMBED_TEX=1 \
        ${INC_PATH} ${SRC_LIST} ${LIB_PATH} ${LIB_LIST} -o RooT.a64_64sve

RooT.a64f32sve:
	clang++ -O3 -g -pthread \
        -DRT_LINUX -DRT_A64 -DRT_128=1 -DRT_256=1+4 -DRT_512=1+4 -DRT_1K4=1+4 \
        -DRT_POINTER=64 -DRT_ADDRESS=64 -DRT_ELEMENT=32 -DRT_ENDIAN=0 \
        -DRT_DEBUG=0 -DRT_PATH="../" -DRT_FULLSCREEN=0 \
        -DRT_EMBED_STDOUT=0 -DRT_EMBED_FILEIO=0 -DRT_EMBED_TEX=1 \
        ${INC_PATH} ${SRC_LIST} ${LIB_PATH} ${LIB_LIST} -o RooT.a64f32sve

RooT.a64f64sve:
	clang++ -O3 -g -pthread \
        -DRT_LINUX -DRT_A64 -DRT_128=1 -DRT_256=1+4 -DRT_512=1+4 -DRT_1K4=1+4 \
        -DRT_POINTER=64 -DRT_ADDRESS=64 -DRT_ELEMENT=64 -DRT_ENDIAN=0 \
        -DRT_DEBUG=0 -DRT_PATH="../" -DRT_FULLSCREEN=0 \
        -DRT_EMBED_STDOUT=0 -DRT_EMBED_FILEIO=0 -DRT_EMBED_TEX=1 \
        ${INC_PATH} ${SRC_LIST} ${LIB_PATH} ${LIB_LIST} -o RooT.a64f64sve


# Prerequisites for the build:
# native-compiler for AArch64 is installed and in the PATH variable.
# sudo apt-get update (on AArch64 host or QEMU system mode)
# sudo apt-get install make g++ libxext-dev
#
# Compiling/running RooT demo:
# make -f RooT_make_a64.mk
# ./RooT.a64f32 (on AArch64 host or QEMU system mode)
# ./RooT.a64f32sve (on AArch64 host with SVE or QEMU system mode)
# (has been tested on Raspberry Pi 3 target host system with Devuan/openSUSE)
# (SIMD and CORE tests pass in QEMU linux-user mode, check test subfolder)

# Clang native build works too (takes much longer prior to 3.8):
# sudo apt-get update (on Ubuntu add "universe" to "main" /etc/apt/sources.list)
# sudo apt-get install clang libxext-dev (on AArch64 host or QEMU system mode)
# make -f RooT_make_a64.mk clean
# make -f RooT_make_a64.mk clang -j4

# RooT demo uses runtime SIMD target selection, multiple can be specified above
# on RISC targets top value above is chosen by default, use -n/-k/-s to override
# 256-bit SIMD is achieved by combining pairs of 128-bit registers/instructions
# For 2048-bit SVE build use (replace): RT_2K8_R8=1+4

# For interpretation of SIMD build flags check compatibility layer in rtzero.h
# or refer to the corresponding simd_make_***.mk file.

# 64/32-bit (ptr/adr) hybrid mode is compatible with native 64-bit ABI,
# use (replace): RT_ADDRESS=32, rename the binary to RooT.a64_**
# 64-bit packed SIMD mode (fp64/int64) is supported on 64-bit targets,
# use (replace): RT_ELEMENT=64, rename the binary to RooT.a64*64
