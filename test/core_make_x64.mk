
INC_PATH =                                  \
        -I../core/config/                   \
        -I../core/engine/                   \
        -I../core/system/                   \
        -I../core/tracer/                   \
        -I../data/materials/                \
        -I../data/objects/                  \
        -I../data/textures/                 \
        -Iscenes/

SRC_LIST =                                  \
        ../core/engine/engine.cpp           \
        ../core/engine/object.cpp           \
        ../core/engine/rtgeom.cpp           \
        ../core/engine/rtimag.cpp           \
        ../core/system/system.cpp           \
        ../core/tracer/tracer.cpp           \
        ../core/tracer/tracer_128v2.cpp     \
        ../core/tracer/tracer_128v4.cpp     \
        ../core/tracer/tracer_128v8.cpp     \
        ../core/tracer/tracer_256v4_r8.cpp  \
        ../core/tracer/tracer_256v1.cpp     \
        ../core/tracer/tracer_256v2.cpp     \
        ../core/tracer/tracer_256v8.cpp     \
        ../core/tracer/tracer_512v1_r8.cpp  \
        ../core/tracer/tracer_512v2_r8.cpp  \
        ../core/tracer/tracer_512v1.cpp     \
        ../core/tracer/tracer_512v2.cpp     \
        ../core/tracer/tracer_512v4.cpp     \
        ../core/tracer/tracer_512v8.cpp     \
        ../core/tracer/tracer_1K4v1.cpp     \
        ../core/tracer/tracer_1K4v2.cpp     \
        ../core/tracer/tracer_2K8v1_r8.cpp  \
        ../core/tracer/tracer_2K8v2_r8.cpp  \
        core_test.cpp

LIB_PATH =

LIB_LIST =                                  \
        -lm                                 \
        -lstdc++


build: core_test_x64_32 core_test_x64_64 core_test_x64f32 core_test_x64f64

strip:
	strip core_test.x64*

clean:
	rm core_test.x64*

macOS:
	mv core_test.x64_32 core_test.o64_32
	mv core_test.x64_64 core_test.o64_64
	mv core_test.x64f32 core_test.o64f32
	mv core_test.x64f64 core_test.o64f64

macRD:
	rm -fr core_test.x64*.dSYM/

macRM:
	rm core_test.o64*


core_test_x64_32:
	g++ -O3 -g \
        -DRT_LINUX -DRT_X64 -DRT_128=2+4+8 -DRT_256_R8=4 -DRT_256=1+2+8 \
        -DRT_512_R8=1+2 -DRT_512=1+2 -DRT_1K4=1+2 -DRT_2K8_R8=0 \
        -DRT_POINTER=64 -DRT_ADDRESS=32 -DRT_ELEMENT=32 -DRT_ENDIAN=0 \
        -DRT_DEBUG=0 -DRT_PATH="../" \
        -DRT_EMBED_STDOUT=0 -DRT_EMBED_FILEIO=0 -DRT_EMBED_TEX=1 \
        ${INC_PATH} ${SRC_LIST} ${LIB_PATH} ${LIB_LIST} -o core_test.x64_32

core_test_x64_64:
	g++ -O3 -g \
        -DRT_LINUX -DRT_X64 -DRT_128=2+4+8 -DRT_256_R8=4 -DRT_256=1+2+8 \
        -DRT_512_R8=1+2 -DRT_512=1+2 -DRT_1K4=1+2 -DRT_2K8_R8=0 \
        -DRT_POINTER=64 -DRT_ADDRESS=32 -DRT_ELEMENT=64 -DRT_ENDIAN=0 \
        -DRT_DEBUG=0 -DRT_PATH="../" \
        -DRT_EMBED_STDOUT=0 -DRT_EMBED_FILEIO=0 -DRT_EMBED_TEX=1 \
        ${INC_PATH} ${SRC_LIST} ${LIB_PATH} ${LIB_LIST} -o core_test.x64_64

core_test_x64f32:
	g++ -O3 -g \
        -DRT_LINUX -DRT_X64 -DRT_128=2+4+8 -DRT_256_R8=4 -DRT_256=1+2+8 \
        -DRT_512_R8=1+2 -DRT_512=1+2 -DRT_1K4=1+2 -DRT_2K8_R8=0 \
        -DRT_POINTER=64 -DRT_ADDRESS=64 -DRT_ELEMENT=32 -DRT_ENDIAN=0 \
        -DRT_DEBUG=0 -DRT_PATH="../" \
        -DRT_EMBED_STDOUT=0 -DRT_EMBED_FILEIO=0 -DRT_EMBED_TEX=1 \
        ${INC_PATH} ${SRC_LIST} ${LIB_PATH} ${LIB_LIST} -o core_test.x64f32

core_test_x64f64:
	g++ -O3 -g \
        -DRT_LINUX -DRT_X64 -DRT_128=2+4+8 -DRT_256_R8=4 -DRT_256=1+2+8 \
        -DRT_512_R8=1+2 -DRT_512=1+2 -DRT_1K4=1+2 -DRT_2K8_R8=0 \
        -DRT_POINTER=64 -DRT_ADDRESS=64 -DRT_ELEMENT=64 -DRT_ENDIAN=0 \
        -DRT_DEBUG=0 -DRT_PATH="../" \
        -DRT_EMBED_STDOUT=0 -DRT_EMBED_FILEIO=0 -DRT_EMBED_TEX=1 \
        ${INC_PATH} ${SRC_LIST} ${LIB_PATH} ${LIB_LIST} -o core_test.x64f64


# Prerequisites for the build:
# native-compiler for x86_64 is installed and in the PATH variable.
# sudo apt-get update
# sudo apt-get install make g++
#
# When building on macOS install Command Line Tools first.
# http://osxdaily.com/2014/02/12/install-command-line-tools-mac-os-x/
#
# Building/running CORE test:
# make -f core_make_x64.mk
# ./core_test.x64f32 -i -a -c 1
# (should produce antialiased "-a" images "-i" in the ../dump subfolder)
# Use "-c 1" option to reduce test time when emulating with Intel SDE

# Clang native build works too (takes much longer prior to 3.8), use (replace):
# clang++ (in place of g++) on Ubuntu add "universe" to /etc/apt/sources.list
# sudo apt-get update
# sudo apt-get install clang

# core_test uses runtime SIMD target selection, multiple can be specified above
# on x86 targets top cpuid-value is chosen by default, use -n/-k/-s to override
# 1K4-bit SIMD is achieved by combining pairs of 512-bit registers/instructions
# 2K8-bit SIMD is achieved by combining quads of 512-bit registers/instructions
# For 30-regs 512-bit AVX512F/DQ targets on Skylake-X use (replace): RT_512=4+8

# For interpretation of SIMD build flags check compatibility layer in rtzero.h
# or refer to the corresponding simd_make_***.mk file.

# 64/32-bit (ptr/adr) hybrid mode is compatible with native 64-bit ABI,
# use (replace): RT_ADDRESS=32, rename the binary to core_test.x64_**
# 64-bit packed SIMD mode (fp64/int64) is supported on 64-bit targets,
# use (replace): RT_ELEMENT=64, rename the binary to core_test.x64*64
