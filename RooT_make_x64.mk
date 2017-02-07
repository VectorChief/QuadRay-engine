
INC_PATH =                              \
        -Icore/config/                  \
        -Icore/engine/                  \
        -Icore/system/                  \
        -Icore/tracer/                  \
        -Idata/materials/               \
        -Idata/objects/                 \
        -Idata/scenes/                  \
        -Idata/textures/

SRC_LIST =                              \
        core/engine/engine.cpp          \
        core/engine/object.cpp          \
        core/engine/rtgeom.cpp          \
        core/engine/rtimag.cpp          \
        core/system/system.cpp          \
        core/tracer/tracer.cpp          \
        core/tracer/tracer_128v2.cpp    \
        core/tracer/tracer_128v4.cpp    \
        core/tracer/tracer_128v8.cpp    \
        core/tracer/tracer_256v1.cpp    \
        core/tracer/tracer_256v2.cpp    \
        core/tracer/tracer_256v8.cpp    \
        core/tracer/tracer_512v1.cpp    \
        core/tracer/tracer_512v2.cpp    \
        core/tracer/tracer_512v8.cpp    \
        core/tracer/tracer_1K4v8.cpp    \
        core/tracer/tracer_2K8v8.cpp    \
        RooT_linux.cpp

LIB_PATH =

LIB_LIST =                              \
        -lm                             \
        -lstdc++                        \
        -lX11                           \
        -lXext                          \
        -lpthread


build: RooT_x64_32 RooT_x64_64 RooT_x64f32 RooT_x64f64

strip:
	x86_64-linux-gnu-strip RooT.x64*

clean:
	rm RooT.x64*


RooT_x64_32:
	x86_64-linux-gnu-g++ -O3 -g \
        -DRT_LINUX -DRT_X64 \
        -DRT_128=2+4+8 -DRT_256=1+2+8 -DRT_512=1+2+8 -DRT_1K4=8 -DRT_2K8=8 \
        -DRT_POINTER=64 -DRT_ADDRESS=32 -DRT_ELEMENT=32 -DRT_ENDIAN=0 \
        -DRT_DEBUG=0 -DRT_PATH="./" -DRT_FULLSCREEN=0 \
        -DRT_EMBED_STDOUT=0 -DRT_EMBED_FILEIO=0 -DRT_EMBED_TEX=1 \
        ${INC_PATH} ${SRC_LIST} ${LIB_PATH} ${LIB_LIST} -o RooT.x64_32

RooT_x64_64:
	x86_64-linux-gnu-g++ -O3 -g \
        -DRT_LINUX -DRT_X64 \
        -DRT_128=2+4+8 -DRT_256=1+2+8 -DRT_512=1+2+8 -DRT_1K4=8 -DRT_2K8=8 \
        -DRT_POINTER=64 -DRT_ADDRESS=32 -DRT_ELEMENT=64 -DRT_ENDIAN=0 \
        -DRT_DEBUG=0 -DRT_PATH="./" -DRT_FULLSCREEN=0 \
        -DRT_EMBED_STDOUT=0 -DRT_EMBED_FILEIO=0 -DRT_EMBED_TEX=1 \
        ${INC_PATH} ${SRC_LIST} ${LIB_PATH} ${LIB_LIST} -o RooT.x64_64

RooT_x64f32:
	x86_64-linux-gnu-g++ -O3 -g \
        -DRT_LINUX -DRT_X64 \
        -DRT_128=2+4+8 -DRT_256=1+2+8 -DRT_512=1+2+8 -DRT_1K4=8 -DRT_2K8=8 \
        -DRT_POINTER=64 -DRT_ADDRESS=64 -DRT_ELEMENT=32 -DRT_ENDIAN=0 \
        -DRT_DEBUG=0 -DRT_PATH="./" -DRT_FULLSCREEN=0 \
        -DRT_EMBED_STDOUT=0 -DRT_EMBED_FILEIO=0 -DRT_EMBED_TEX=1 \
        ${INC_PATH} ${SRC_LIST} ${LIB_PATH} ${LIB_LIST} -o RooT.x64f32

RooT_x64f64:
	x86_64-linux-gnu-g++ -O3 -g \
        -DRT_LINUX -DRT_X64 \
        -DRT_128=2+4+8 -DRT_256=1+2+8 -DRT_512=1+2+8 -DRT_1K4=8 -DRT_2K8=8 \
        -DRT_POINTER=64 -DRT_ADDRESS=64 -DRT_ELEMENT=64 -DRT_ENDIAN=0 \
        -DRT_DEBUG=0 -DRT_PATH="./" -DRT_FULLSCREEN=0 \
        -DRT_EMBED_STDOUT=0 -DRT_EMBED_FILEIO=0 -DRT_EMBED_TEX=1 \
        ${INC_PATH} ${SRC_LIST} ${LIB_PATH} ${LIB_LIST} -o RooT.x64f64


# Prerequisites for the build:
# native-compiler for x86_64 is installed and in the PATH variable.
# sudo apt-get install g++ libxext-dev (on x86_64 host)
#
# Building/running RooT demo:
# make -f RooT_make_x64.mk
# ./RooT.x64f32

# RooT demo uses runtime SIMD target selection, multiple can be specified above

# Clang compilation works too (takes much longer prior to 3.8), use (replace):
# clang++ -Wno-logical-op-parentheses -Wno-bitwise-op-parentheses
# sudo apt-get install clang

# 64/32-bit (ptr/adr) hybrid mode is compatible with native 64-bit ABI,
# use (replace): RT_ADDRESS=32, rename the binary to RooT.x64_**

# 64-bit packed SIMD mode (fp64/int64) is supported on 64-bit targets,
# use (replace): RT_ELEMENT=64, rename the binary to RooT.x64*64
