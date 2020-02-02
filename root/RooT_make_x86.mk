
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
        ../core/tracer/tracer_128v2.cpp     \
        ../core/tracer/tracer_128v4.cpp     \
        ../core/tracer/tracer_128v8.cpp     \
        ../core/tracer/tracer_256v1.cpp     \
        ../core/tracer/tracer_256v2.cpp     \
        ../core/tracer/tracer_512v1.cpp     \
        ../core/tracer/tracer_512v2.cpp     \
        RooT_linux.cpp

LIB_PATH =                                  \
        -L/usr/X11/lib/

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

macOS:
	mv RooT.x86 RooT.o86

macRD:
	rm -fr RooT.x86*.dSYM/

macRM:
	rm RooT.o86*


RooT_x86:
	g++ -O3 -g -pthread -m32 \
        -DRT_LINUX -DRT_X86 -DRT_128=1+2+4+8 -DRT_256=1+2 -DRT_512=1+2 \
        -DRT_POINTER=32 -DRT_ADDRESS=32 -DRT_ELEMENT=32 -DRT_ENDIAN=0 \
        -DRT_DEBUG=0 -DRT_PATH="../" -DRT_FULLSCREEN=0 \
        -DRT_EMBED_STDOUT=0 -DRT_EMBED_FILEIO=0 -DRT_EMBED_TEX=1 \
        ${INC_PATH} ${SRC_LIST} ${LIB_PATH} ${LIB_LIST} -o RooT.x86


clang: RooT.x86

RooT.x86:
	clang++ -O3 -g -pthread -m32 \
        -DRT_LINUX -DRT_X86 -DRT_128=1+2+4+8 -DRT_256=1+2 -DRT_512=1+2 \
        -DRT_POINTER=32 -DRT_ADDRESS=32 -DRT_ELEMENT=32 -DRT_ENDIAN=0 \
        -DRT_DEBUG=0 -DRT_PATH="../" -DRT_FULLSCREEN=0 \
        -DRT_EMBED_STDOUT=0 -DRT_EMBED_FILEIO=0 -DRT_EMBED_TEX=1 \
        ${INC_PATH} ${SRC_LIST} ${LIB_PATH} ${LIB_LIST} -o RooT.x86


# Prerequisites for the build:
# native/multilib-compiler for x86/x86_64 is installed and in the PATH variable.
# sudo apt-get update
# sudo apt-get install make g++ libxext-dev (x86 host) or if libs are present:
# sudo apt-get install make g++-multilib libxext-dev:i386 (x86_64 host, Mint 18)
# (installation of g++-multilib removes any g++ cross-compilers)
#
# When building on macOS install Command Line Tools and XQuartz first.
# http://osxdaily.com/2014/02/12/install-command-line-tools-mac-os-x/
# https://www.youtube.com/watch?v=uS4zTqfwSSQ  https://www.xquartz.org/
# As pthread affinity features are not supported on a Mac, use "-t n" option
# when running produced binary (below), where "n" is the number of CPU cores.
# Otherwise default maximum number of threads (120) will be created.
#
# Compiling/running RooT demo:
# make -f RooT_make_x86.mk
# ./RooT.x86

# Clang native build works too (takes much longer prior to 3.8):
# sudo apt-get update (on Ubuntu add "universe" to "main" /etc/apt/sources.list)
# sudo apt-get install clang libxext-dev (on x86 host) or if libs are present:
# sudo apt-get install clang g++-multilib libxext-dev:i386 (on x86_64, Mint 18)
# (installation of g++-multilib removes any g++ cross-compilers)
# make -f RooT_make_x86.mk clean
# make -f RooT_make_x86.mk clang

# RooT demo uses runtime SIMD target selection, multiple can be specified above
# on x86 targets top cpuid-value is chosen by default, use -n/-k/-s to override

# For interpretation of SIMD build flags check compatibility layer in rtzero.h
# or refer to the corresponding simd_make_***.mk file.
