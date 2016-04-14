
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
        RooT_linux.cpp

LIB_PATH =

LIB_LIST =                              \
        -lm                             \
        -lstdc++                        \
        -lX11                           \
        -lXext                          \
        -lpthread

RooT:
	aarch64-linux-gnu-g++ -O3 -g -mabi=ilp32 \
        -DRT_LINUX -DRT_A32 -DRT_128=1 \
        -DRT_POINTER=32 -DRT_ADDRESS=32 -DRT_ENDIAN=0 \
        -DRT_DEBUG=0 -DRT_PATH="./" -DRT_FULLSCREEN=0 \
        -DRT_EMBED_STDOUT=0 -DRT_EMBED_FILEIO=0 -DRT_EMBED_TEX=1 \
        ${INC_PATH} ${SRC_LIST} ${LIB_PATH} ${LIB_LIST} -o RooT.a32

# Prerequisites for the build:
# (cross-)compiler for AArch64 is installed and in the PATH variable.
#
# make -f RooT_make_a32.mk

# The ilp32 ABI hasn't yet been tested due to lack of available toolchains/libs.
# The (default) lp64 ABI isn't yet supported by the ./core.
