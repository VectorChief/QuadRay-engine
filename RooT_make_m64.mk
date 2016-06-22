
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
	mips64el-linux-gnuabi64-g++ -O3 -g -mips64r6 -mabi=64 -mmsa \
        -DRT_LINUX -DRT_M64=6 -DRT_128=1 \
        -DRT_POINTER=64 -DRT_ADDRESS=32 -DRT_ENDIAN=0 \
        -DRT_DEBUG=0 -DRT_PATH="./" -DRT_FULLSCREEN=0 \
        -DRT_EMBED_STDOUT=0 -DRT_EMBED_FILEIO=0 -DRT_EMBED_TEX=1 \
        ${INC_PATH} ${SRC_LIST} ${LIB_PATH} ${LIB_LIST} -o RooT.m64_32

# Prerequisites for the build:
# native-compiler for MIPS+MSA is installed and in the PATH variable.
# sudo apt-get install g++ libxext-dev (on I6400 host or QEMU system mode)
# (recent upstream g++ versions for MIPS64 may not fully support MSA)
#
# Building/running RooT demo:
# make -f RooT_make_m64.mk
# ./RooT.m64_32 (on I6400 host or QEMU system mode)
# (hasn't been verified due to lack of target host system)
# (SIMD and CORE tests pass in QEMU linux-user mode, check test subfolder)

# Experimental 64/32-bit hybrid mode is enabled by default
# until full 64-bit support is implemented in the framework.
