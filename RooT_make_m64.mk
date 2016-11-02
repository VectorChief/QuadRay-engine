
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
        core/tracer/tracer_256v1.cpp    \
        RooT_linux.cpp

LIB_PATH =

LIB_LIST =                              \
        -lm                             \
        -lstdc++                        \
        -lX11                           \
        -lXext                          \
        -lpthread


build: RooT_m64_32 RooT_m64_64 RooT_m64f32 RooT_m64f64

strip:
	mips64el-linux-gnuabi64-strip RooT.m64*

clean:
	rm RooT.m64*


RooT_m64_32:
	mips64el-linux-gnuabi64-g++ -O3 -g -mips64r6 -mabi=64 -mmsa \
        -DRT_LINUX -DRT_M64=6 -DRT_128=1 -DRT_256=1 \
        -DRT_POINTER=64 -DRT_ADDRESS=32 -DRT_ELEMENT=32 -DRT_ENDIAN=0 \
        -DRT_DEBUG=0 -DRT_PATH="./" -DRT_FULLSCREEN=0 \
        -DRT_EMBED_STDOUT=0 -DRT_EMBED_FILEIO=0 -DRT_EMBED_TEX=1 \
        ${INC_PATH} ${SRC_LIST} ${LIB_PATH} ${LIB_LIST} -o RooT.m64_32

RooT_m64_64:
	mips64el-linux-gnuabi64-g++ -O3 -g -mips64r6 -mabi=64 -mmsa \
        -DRT_LINUX -DRT_M64=6 -DRT_128=1 -DRT_256=1 \
        -DRT_POINTER=64 -DRT_ADDRESS=32 -DRT_ELEMENT=64 -DRT_ENDIAN=0 \
        -DRT_DEBUG=0 -DRT_PATH="./" -DRT_FULLSCREEN=0 \
        -DRT_EMBED_STDOUT=0 -DRT_EMBED_FILEIO=0 -DRT_EMBED_TEX=1 \
        ${INC_PATH} ${SRC_LIST} ${LIB_PATH} ${LIB_LIST} -o RooT.m64_64

RooT_m64f32:
	mips64el-linux-gnuabi64-g++ -O3 -g -mips64r6 -mabi=64 -mmsa \
        -DRT_LINUX -DRT_M64=6 -DRT_128=1 -DRT_256=1 \
        -DRT_POINTER=64 -DRT_ADDRESS=64 -DRT_ELEMENT=32 -DRT_ENDIAN=0 \
        -DRT_DEBUG=0 -DRT_PATH="./" -DRT_FULLSCREEN=0 \
        -DRT_EMBED_STDOUT=0 -DRT_EMBED_FILEIO=0 -DRT_EMBED_TEX=1 \
        ${INC_PATH} ${SRC_LIST} ${LIB_PATH} ${LIB_LIST} -o RooT.m64f32

RooT_m64f64:
	mips64el-linux-gnuabi64-g++ -O3 -g -mips64r6 -mabi=64 -mmsa \
        -DRT_LINUX -DRT_M64=6 -DRT_128=1 -DRT_256=1 \
        -DRT_POINTER=64 -DRT_ADDRESS=64 -DRT_ELEMENT=64 -DRT_ENDIAN=0 \
        -DRT_DEBUG=0 -DRT_PATH="./" -DRT_FULLSCREEN=0 \
        -DRT_EMBED_STDOUT=0 -DRT_EMBED_FILEIO=0 -DRT_EMBED_TEX=1 \
        ${INC_PATH} ${SRC_LIST} ${LIB_PATH} ${LIB_LIST} -o RooT.m64f64


# Prerequisites for the build:
# native-compiler for MIPS+MSA is installed and in the PATH variable.
# sudo apt-get install g++ libxext-dev (on I6400 host or QEMU system mode)
# (recent upstream g++-5-mips64el series may not fully support MSA)
#
# Building/running RooT demo:
# make -f RooT_make_m64.mk
# ./RooT.m64f32 (on I6400 host or QEMU system mode)
# (hasn't been verified due to lack of target host system)
# (SIMD and CORE tests pass in QEMU linux-user mode, check test subfolder)

# For 256-bit NEON build use (replace): RT_256=1 (uses pairs of regs/ops)

# 64/32-bit (ptr/adr) hybrid mode is compatible with native 64-bit ABI,
# use (replace): RT_ADDRESS=32, rename the binary to RooT.m64_**

# 64-bit packed SIMD mode (fp64/int64) is supported on 64-bit targets,
# use (replace): RT_ELEMENT=64, rename the binary to RooT.m64*64
