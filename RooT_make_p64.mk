
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


build: RooT_p64_32 RooT_p64_64 RooT_p64f32 RooT_p64f64

strip:
	powerpc64le-linux-gnu-strip RooT.p64*

clean:
	rm RooT.p64*


RooT_p64_32:
	powerpc64le-linux-gnu-g++ -O2 -g \
        -DRT_LINUX -DRT_P64 -DRT_128=2+4 -DRT_256=1+2 \
        -DRT_POINTER=64 -DRT_ADDRESS=32 -DRT_ELEMENT=32 -DRT_ENDIAN=0 \
        -DRT_DEBUG=0 -DRT_PATH="./" -DRT_FULLSCREEN=0 \
        -DRT_EMBED_STDOUT=0 -DRT_EMBED_FILEIO=0 -DRT_EMBED_TEX=1 \
        ${INC_PATH} ${SRC_LIST} ${LIB_PATH} ${LIB_LIST} -o RooT.p64_32

RooT_p64_64:
	powerpc64le-linux-gnu-g++ -O2 -g \
        -DRT_LINUX -DRT_P64 -DRT_128=2+4 -DRT_256=1+2 \
        -DRT_POINTER=64 -DRT_ADDRESS=32 -DRT_ELEMENT=64 -DRT_ENDIAN=0 \
        -DRT_DEBUG=0 -DRT_PATH="./" -DRT_FULLSCREEN=0 \
        -DRT_EMBED_STDOUT=0 -DRT_EMBED_FILEIO=0 -DRT_EMBED_TEX=1 \
        ${INC_PATH} ${SRC_LIST} ${LIB_PATH} ${LIB_LIST} -o RooT.p64_64

RooT_p64f32:
	powerpc64le-linux-gnu-g++ -O2 -g \
        -DRT_LINUX -DRT_P64 -DRT_128=2+4 -DRT_256=1+2 \
        -DRT_POINTER=64 -DRT_ADDRESS=64 -DRT_ELEMENT=32 -DRT_ENDIAN=0 \
        -DRT_DEBUG=0 -DRT_PATH="./" -DRT_FULLSCREEN=0 \
        -DRT_EMBED_STDOUT=0 -DRT_EMBED_FILEIO=0 -DRT_EMBED_TEX=1 \
        ${INC_PATH} ${SRC_LIST} ${LIB_PATH} ${LIB_LIST} -o RooT.p64f32

RooT_p64f64:
	powerpc64le-linux-gnu-g++ -O2 -g \
        -DRT_LINUX -DRT_P64 -DRT_128=2+4 -DRT_256=1+2 \
        -DRT_POINTER=64 -DRT_ADDRESS=64 -DRT_ELEMENT=64 -DRT_ENDIAN=0 \
        -DRT_DEBUG=0 -DRT_PATH="./" -DRT_FULLSCREEN=0 \
        -DRT_EMBED_STDOUT=0 -DRT_EMBED_FILEIO=0 -DRT_EMBED_TEX=1 \
        ${INC_PATH} ${SRC_LIST} ${LIB_PATH} ${LIB_LIST} -o RooT.p64f64


# Prerequisites for the build:
# native-compiler for 64-bit Power is installed and in the PATH variable.
# sudo apt-get install g++ libxext-dev (on POWER8 host or QEMU system mode)
# (recent g++-5-powerpc64le series target POWER8 and don't work well with -O3)
#
# Building/running RooT demo:
# make -f RooT_make_p64.mk
# ./RooT.p64f32 (on POWER8 host or QEMU system mode)
# (hasn't been verified due to lack of target host system)
# (SIMD and CORE tests pass in QEMU linux-user mode, check test subfolder)

# For 256-bit VSX1 build use (replace): RT_256=1 (uses pairs of regs/ops)
# For 256-bit VSX2 build use (replace): RT_256=2 (uses pairs of regs/ops)

# For 128-bit VSX1 POWER(7,7+,8) big-endian target use (replace):
# powerpc64-linux-gnu-g++ -O3 -DRT_128=2 -DRT_ENDIAN=1

# 64/32-bit (ptr/adr) hybrid mode is compatible with native 64-bit ABI,
# use (replace): RT_ADDRESS=32, rename the binary to RooT.p64_**

# 64-bit packed SIMD mode (fp64/int64) is supported on 64-bit targets,
# use (replace): RT_ELEMENT=64, rename the binary to RooT.p64*64
