
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
        core/tracer/tracer_128v1.cpp    \
        core/tracer/tracer_128v2.cpp    \
        core/tracer/tracer_128v4.cpp    \
        core/tracer/tracer_256v1.cpp    \
        core/tracer/tracer_256v2.cpp    \
        RooT_linux.cpp

LIB_PATH =

LIB_LIST =                              \
        -lm                             \
        -lstdc++                        \
        -lX11                           \
        -lXext                          \
        -lpthread

RooT:
	x86_64-linux-gnu-g++ -O3 -g \
        -DRT_LINUX -DRT_X64 -DRT_128=1+2+4 -DRT_256=1+2 \
        -DRT_POINTER=64 -DRT_ADDRESS=64 -DRT_ENDIAN=0 \
        -DRT_DEBUG=0 -DRT_PATH="./" -DRT_FULLSCREEN=0 \
        -DRT_EMBED_STDOUT=0 -DRT_EMBED_FILEIO=0 -DRT_EMBED_TEX=1 \
        ${INC_PATH} ${SRC_LIST} ${LIB_PATH} ${LIB_LIST} -o RooT.x64f32

# Prerequisites for the build:
# native-compiler for x86_64 is installed and in the PATH variable.
# sudo apt-get install g++ libxext-dev (on x86_64 host)
#
# Building/running RooT demo:
# make -f RooT_make_x64.mk
# ./RooT.x64f32

# Clang compilation works too (takes much longer prior to 3.8), use (replace):
# clang++ -Wno-logical-op-parentheses -Wno-bitwise-op-parentheses
# sudo apt-get install clang

# Experimental 64/32-bit hybrid mode compatible with native 64-bit ABI
# is available for the original pure 32-bit ISA using 64-bit pointers,
# use (replace): RT_ADDRESS=32, rename the binary to RooT.x64_32
