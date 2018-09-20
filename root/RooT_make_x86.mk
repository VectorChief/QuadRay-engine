
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
        ../core/tracer/tracer_128v2.cpp     \
        ../core/tracer/tracer_128v4.cpp     \
        ../core/tracer/tracer_128v8.cpp     \
        ../core/tracer/tracer_256v1.cpp     \
        ../core/tracer/tracer_256v2.cpp     \
        ../core/tracer/tracer_512v1.cpp     \
        ../core/tracer/tracer_512v2.cpp     \
        RooT_linux.cpp

LIB_PATH =

LIB_LIST =                                  \
        -lm                                 \
        -lstdc++                            \
        -lX11                               \
        -lXext                              \
        -lpthread


build: RooT_x86

strip:
	strip RooT.x86*

clean:
	rm RooT.x86*


RooT_x86:
	g++ -O3 -g -pthread -no-pie -m32 \
        -DRT_LINUX -DRT_X86 -DRT_128=1+2+4+8 -DRT_256=1+2 -DRT_512=1+2 \
        -DRT_POINTER=32 -DRT_ADDRESS=32 -DRT_ELEMENT=32 -DRT_ENDIAN=0 \
        -DRT_DEBUG=0 -DRT_PATH="../" -DRT_FULLSCREEN=0 \
        -DRT_EMBED_STDOUT=0 -DRT_EMBED_FILEIO=0 -DRT_EMBED_TEX=1 \
        ${INC_PATH} ${SRC_LIST} ${LIB_PATH} ${LIB_LIST} -o RooT.x86


clang: RooT.x86

RooT.x86:
	clang++ -O3 -g -pthread -m32 \
        -Wno-unknown-warning-option \
        -Wno-shift-negative-value -Wno-shift-op-parentheses \
        -Wno-logical-op-parentheses -Wno-bitwise-op-parentheses \
        -DRT_LINUX -DRT_X86 -DRT_128=1+2+4+8 -DRT_256=1+2 -DRT_512=1+2 \
        -DRT_POINTER=32 -DRT_ADDRESS=32 -DRT_ELEMENT=32 -DRT_ENDIAN=0 \
        -DRT_DEBUG=0 -DRT_PATH="../" -DRT_FULLSCREEN=0 \
        -DRT_EMBED_STDOUT=0 -DRT_EMBED_FILEIO=0 -DRT_EMBED_TEX=1 \
        ${INC_PATH} ${SRC_LIST} ${LIB_PATH} ${LIB_LIST} -o RooT.x86


# Prerequisites for the build:
# native/multilib-compiler for x86/x86_64 is installed and in the PATH variable.
# sudo apt-get update
# sudo apt-get install g++ libxext-dev (on x86 host) or if libs are present:
# sudo apt-get install g++-multilib libxext-dev:i386 (on x86_64 host, Mint 18)
# (installation of g++-multilib may remove g++ cross-compilers on some distros)
#
# Building/running RooT demo:
# make -f RooT_make_x86.mk
# ./RooT.x86

# g++ compilation works on Debian/Ubuntu without PIE-mode (-no-pie >= g++-5)
# clang compilation takes much longer prior to 3.8 (older Ubuntu 14.04/Mint 17)
# sudo apt-get update (on Ubuntu add "universe" to "main" /etc/apt/sources.list)
# sudo apt-get install clang libxext-dev (on x86 host) or if libs are present:
# sudo apt-get install clang g++-multilib libxext-dev:i386 (on x86_64, Mint 18)
# make -f RooT_make_x86.mk clang

# RooT demo uses runtime SIMD target selection, multiple can be specified above
# on x86 targets top cpuid-value is chosen by default, use -n/-k/-s to override

# For interpretation of SIMD build flags check compatibility layer in rtzero.h
