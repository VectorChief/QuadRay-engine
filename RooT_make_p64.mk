
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
        RooT_linux.cpp

LIB_PATH =

LIB_LIST =                              \
        -lm                             \
        -lstdc++                        \
        -lX11                           \
        -lXext                          \
        -lpthread

RooT:
	powerpc64le-linux-gnu-g++ -O2 -g \
        -DRT_LINUX -DRT_P64 -DRT_128=1+2 \
        -DRT_POINTER=64 -DRT_ADDRESS=32 -DRT_ENDIAN=0 \
        -DRT_DEBUG=0 -DRT_PATH="./" -DRT_FULLSCREEN=0 \
        -DRT_EMBED_STDOUT=0 -DRT_EMBED_FILEIO=0 -DRT_EMBED_TEX=1 \
        ${INC_PATH} ${SRC_LIST} ${LIB_PATH} ${LIB_LIST} -o RooT.p64_32

# Prerequisites for the build:
# (cross-)compiler for PowerPC is installed and in the PATH variable.
#
# make -f RooT_make_p64.mk

# For 64-bit Power(7,7+,8) VMX/VSX big-endian target use (replace):
# powerpc64-linux-gnu-g++ -DRT_ENDIAN=1

# Experimental 64/32-bit hybrid mode is enabled by default
# until full 64-bit support is implemented in the framework.