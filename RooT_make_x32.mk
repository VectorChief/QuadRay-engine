
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
        core/tracer/tracer_128v8.cpp    \
        core/tracer/tracer_256v1.cpp    \
        core/tracer/tracer_256v2.cpp    \
        core/tracer/tracer_256v8.cpp    \
        core/tracer/tracer_512v1.cpp    \
        core/tracer/tracer_512v2.cpp    \
        core/tracer/tracer_512v8.cpp    \
        core/tracer/tracer_1K4v1.cpp    \
        core/tracer/tracer_1K4v2.cpp    \
        core/tracer/tracer_2K8v8.cpp    \
        RooT_linux.cpp

LIB_PATH =

LIB_LIST =                              \
        -lm                             \
        -lstdc++                        \
        -lX11                           \
        -lXext                          \
        -lpthread


build: RooT_x32

strip:
	x86_64-linux-gnu-strip RooT.x32

clean:
	rm RooT.x32


RooT_x32:
	x86_64-linux-gnu-g++ -O3 -g -mx32 \
        -DRT_LINUX -DRT_X32 \
        -DRT_128=1+2+4+8 -DRT_256=1+2+8 -DRT_512=1+2+8 -DRT_1K4=1+2 -DRT_2K8=0 \
        -DRT_POINTER=32 -DRT_ADDRESS=32 -DRT_ELEMENT=32 -DRT_ENDIAN=0 \
        -DRT_DEBUG=0 -DRT_PATH="./" -DRT_FULLSCREEN=0 \
        -DRT_EMBED_STDOUT=0 -DRT_EMBED_FILEIO=0 -DRT_EMBED_TEX=1 \
        ${INC_PATH} ${SRC_LIST} ${LIB_PATH} ${LIB_LIST} -o RooT.x32


# Prerequisites for the build:
# multilib-compiler for x86_64 is installed and in the PATH variable.
# sudo apt-get install g++-multilib (plus X11/Xext libs for x32 ABI)
# (installation of g++-multilib removes any g++ cross-compilers)
#
# Building/running RooT demo:
# make -f RooT_make_x32.mk
# ./RooT.x32

# RooT demo uses runtime SIMD target selection, multiple can be specified above

# Clang compilation works too (takes much longer prior to 3.8), use (replace):
# clang++ -Wno-logical-op-parentheses -Wno-bitwise-op-parentheses
# sudo apt-get install clang (requires g++-multilib for non-native ABI)

# 32-bit ABI hasn't been fully tested yet due to lack of available libs,
# check out 64/32-bit (ptr/adr) hybrid mode for 64-bit ABI in RooT_make_x64.mk
