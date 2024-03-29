
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
        ../core/tracer/tracer_128v1.cpp     \
        ../core/tracer/tracer_128v2.cpp     \
        ../core/tracer/tracer_128v4.cpp     \
        ../core/tracer/tracer_256v1.cpp     \
        ../core/tracer/tracer_256v2.cpp     \
        ../core/tracer/tracer_256v4.cpp     \
        ../core/tracer/tracer_256v8.cpp     \
        ../core/tracer/tracer_512v1.cpp     \
        ../core/tracer/tracer_512v2.cpp     \
        core_test.cpp

LIB_PATH =

LIB_LIST =                                  \
        -lm                                 \
        -lstdc++


build: build_p9 build_le build_be

strip:
	powerpc64le-linux-gnu-strip core_test.p64???L*
	powerpc64-linux-gnu-strip core_test.p64???B*

clean:
	rm core_test.p64*


# using -mcpu=power8 for power9 targets is a workaround for QEMU 6.2.0 bug
# https://bugs.launchpad.net/ubuntu/+source/qemu/+bug/2011832

build_p9: core_test_p64_32Lp9 core_test_p64_64Lp9 \
          core_test_p64f32Lp9 core_test_p64f64Lp9

core_test_p64_32Lp9:
	powerpc64le-linux-gnu-g++ -O2 -g -static -mcpu=power8 \
        -DRT_LINUX -DRT_P64 -DRT_128=1+2 -DRT_256=1+2 \
        -DRT_POINTER=64 -DRT_ADDRESS=32 -DRT_ELEMENT=32 -DRT_ENDIAN=0 \
        -DRT_DEBUG=0 -DRT_PATH="../" \
        -DRT_EMBED_STDOUT=0 -DRT_EMBED_FILEIO=0 -DRT_EMBED_TEX=1 \
        ${INC_PATH} ${SRC_LIST} ${LIB_PATH} ${LIB_LIST} -o core_test.p64_32Lp9

core_test_p64_64Lp9:
	powerpc64le-linux-gnu-g++ -O2 -g -static -mcpu=power8 \
        -DRT_LINUX -DRT_P64 -DRT_128=1+2 -DRT_256=1+2 \
        -DRT_POINTER=64 -DRT_ADDRESS=32 -DRT_ELEMENT=64 -DRT_ENDIAN=0 \
        -DRT_DEBUG=0 -DRT_PATH="../" \
        -DRT_EMBED_STDOUT=0 -DRT_EMBED_FILEIO=0 -DRT_EMBED_TEX=1 \
        ${INC_PATH} ${SRC_LIST} ${LIB_PATH} ${LIB_LIST} -o core_test.p64_64Lp9

core_test_p64f32Lp9:
	powerpc64le-linux-gnu-g++ -O2 -g -static -mcpu=power8 \
        -DRT_LINUX -DRT_P64 -DRT_128=1+2 -DRT_256=1+2 \
        -DRT_POINTER=64 -DRT_ADDRESS=64 -DRT_ELEMENT=32 -DRT_ENDIAN=0 \
        -DRT_DEBUG=0 -DRT_PATH="../" \
        -DRT_EMBED_STDOUT=0 -DRT_EMBED_FILEIO=0 -DRT_EMBED_TEX=1 \
        ${INC_PATH} ${SRC_LIST} ${LIB_PATH} ${LIB_LIST} -o core_test.p64f32Lp9

core_test_p64f64Lp9:
	powerpc64le-linux-gnu-g++ -O2 -g -static -mcpu=power8 \
        -DRT_LINUX -DRT_P64 -DRT_128=1+2 -DRT_256=1+2 \
        -DRT_POINTER=64 -DRT_ADDRESS=64 -DRT_ELEMENT=64 -DRT_ENDIAN=0 \
        -DRT_DEBUG=0 -DRT_PATH="../" \
        -DRT_EMBED_STDOUT=0 -DRT_EMBED_FILEIO=0 -DRT_EMBED_TEX=1 \
        ${INC_PATH} ${SRC_LIST} ${LIB_PATH} ${LIB_LIST} -o core_test.p64f64Lp9


build_le: core_test_p64_32Lp8 core_test_p64_64Lp8 \
          core_test_p64f32Lp8 core_test_p64f64Lp8

core_test_p64_32Lp8:
	powerpc64le-linux-gnu-g++ -O2 -g -static -mcpu=power8 \
        -DRT_LINUX -DRT_P64 -DRT_128=1 -DRT_256=1 \
        -DRT_POINTER=64 -DRT_ADDRESS=32 -DRT_ELEMENT=32 -DRT_ENDIAN=0 \
        -DRT_DEBUG=0 -DRT_PATH="../" \
        -DRT_EMBED_STDOUT=0 -DRT_EMBED_FILEIO=0 -DRT_EMBED_TEX=1 \
        ${INC_PATH} ${SRC_LIST} ${LIB_PATH} ${LIB_LIST} -o core_test.p64_32Lp8

core_test_p64_64Lp8:
	powerpc64le-linux-gnu-g++ -O2 -g -static -mcpu=power8 \
        -DRT_LINUX -DRT_P64 -DRT_128=1 -DRT_256=1 \
        -DRT_POINTER=64 -DRT_ADDRESS=32 -DRT_ELEMENT=64 -DRT_ENDIAN=0 \
        -DRT_DEBUG=0 -DRT_PATH="../" \
        -DRT_EMBED_STDOUT=0 -DRT_EMBED_FILEIO=0 -DRT_EMBED_TEX=1 \
        ${INC_PATH} ${SRC_LIST} ${LIB_PATH} ${LIB_LIST} -o core_test.p64_64Lp8

core_test_p64f32Lp8:
	powerpc64le-linux-gnu-g++ -O2 -g -static -mcpu=power8 \
        -DRT_LINUX -DRT_P64 -DRT_128=1 -DRT_256=1 \
        -DRT_POINTER=64 -DRT_ADDRESS=64 -DRT_ELEMENT=32 -DRT_ENDIAN=0 \
        -DRT_DEBUG=0 -DRT_PATH="../" \
        -DRT_EMBED_STDOUT=0 -DRT_EMBED_FILEIO=0 -DRT_EMBED_TEX=1 \
        ${INC_PATH} ${SRC_LIST} ${LIB_PATH} ${LIB_LIST} -o core_test.p64f32Lp8

core_test_p64f64Lp8:
	powerpc64le-linux-gnu-g++ -O2 -g -static -mcpu=power8 \
        -DRT_LINUX -DRT_P64 -DRT_128=1 -DRT_256=1 \
        -DRT_POINTER=64 -DRT_ADDRESS=64 -DRT_ELEMENT=64 -DRT_ENDIAN=0 \
        -DRT_DEBUG=0 -DRT_PATH="../" \
        -DRT_EMBED_STDOUT=0 -DRT_EMBED_FILEIO=0 -DRT_EMBED_TEX=1 \
        ${INC_PATH} ${SRC_LIST} ${LIB_PATH} ${LIB_LIST} -o core_test.p64f64Lp8


build_be: core_test_p64_32Bp7 core_test_p64_64Bp7 \
          core_test_p64f32Bp7 core_test_p64f64Bp7

core_test_p64_32Bp7:
	powerpc64-linux-gnu-g++ -O2 -g -static \
        -DRT_LINUX -DRT_P64 -DRT_128=1 -DRT_256=1 \
        -DRT_POINTER=64 -DRT_ADDRESS=32 -DRT_ELEMENT=32 -DRT_ENDIAN=1 \
        -DRT_DEBUG=0 -DRT_PATH="../" \
        -DRT_EMBED_STDOUT=0 -DRT_EMBED_FILEIO=0 -DRT_EMBED_TEX=1 \
        ${INC_PATH} ${SRC_LIST} ${LIB_PATH} ${LIB_LIST} -o core_test.p64_32Bp7

core_test_p64_64Bp7:
	powerpc64-linux-gnu-g++ -O2 -g -static \
        -DRT_LINUX -DRT_P64 -DRT_128=1 -DRT_256=1 \
        -DRT_POINTER=64 -DRT_ADDRESS=32 -DRT_ELEMENT=64 -DRT_ENDIAN=1 \
        -DRT_DEBUG=0 -DRT_PATH="../" \
        -DRT_EMBED_STDOUT=0 -DRT_EMBED_FILEIO=0 -DRT_EMBED_TEX=1 \
        ${INC_PATH} ${SRC_LIST} ${LIB_PATH} ${LIB_LIST} -o core_test.p64_64Bp7

core_test_p64f32Bp7:
	powerpc64-linux-gnu-g++ -O2 -g -static \
        -DRT_LINUX -DRT_P64 -DRT_128=1 -DRT_256=1 \
        -DRT_POINTER=64 -DRT_ADDRESS=64 -DRT_ELEMENT=32 -DRT_ENDIAN=1 \
        -DRT_DEBUG=0 -DRT_PATH="../" \
        -DRT_EMBED_STDOUT=0 -DRT_EMBED_FILEIO=0 -DRT_EMBED_TEX=1 \
        ${INC_PATH} ${SRC_LIST} ${LIB_PATH} ${LIB_LIST} -o core_test.p64f32Bp7

core_test_p64f64Bp7:
	powerpc64-linux-gnu-g++ -O2 -g -static \
        -DRT_LINUX -DRT_P64 -DRT_128=1 -DRT_256=1 \
        -DRT_POINTER=64 -DRT_ADDRESS=64 -DRT_ELEMENT=64 -DRT_ENDIAN=1 \
        -DRT_DEBUG=0 -DRT_PATH="../" \
        -DRT_EMBED_STDOUT=0 -DRT_EMBED_FILEIO=0 -DRT_EMBED_TEX=1 \
        ${INC_PATH} ${SRC_LIST} ${LIB_PATH} ${LIB_LIST} -o core_test.p64f64Bp7


# On Ubuntu (MATE) 16.04-22.04 add "universe multiverse" to "main restricted"
# in /etc/apt/sources.list (sudo nano /etc/apt/sources.list) then run:
# sudo apt-get update
# (Ubuntu MATE is set up for an update without a need to edit the file)
# (extended repositories "universe multiverse" are only needed for clang)
#
# Prerequisites for the build:
# (cross-)compiler for 64-bit POWER is installed and in the PATH variable.
# sudo apt-get install make g++-powerpc64le-linux-gnu
# sudo apt-get install make g++-powerpc64-linux-gnu
# (recent g++-5-powerpc64le series target POWER8 and don't work well with -O3)
#
# Prerequisites for emulation:
# recent QEMU(-2.5) is installed or built from source and in the PATH variable.
# POWER9 target requires more recent QEMU, tested with 3.x.y series and 4.2.0.
# QEMU versions 4.x.y prior to 4.2.0 show issues with POWER8/9 fp32 LE targets.
# sudo apt-get install qemu-user
#
# Compiling/running CORE test:
# make -f core_make_p64.mk
# qemu-ppc64le -cpu POWER9 core_test.p64f32Lp9 -i -a -c 1
# qemu-ppc64le -cpu POWER8 core_test.p64f32Lp8 -i -a -c 1 (POWER9, Ubuntu 22.04)
# qemu-ppc64   -cpu POWER7 core_test.p64f32Bp7 -i -a -c 1
# (should produce antialiased "-a" images "-i" in the ../dump subfolder)
# Use "-c 1" option to reduce test time when emulating with QEMU

# Clang native build works too (takes much longer prior to 3.8), use (replace):
# clang++ -O0 (in place of ...-g++ -O2) on 64-bit POWER host (Tyan TN71-BP012)
# sudo apt-get install clang

# core_test uses runtime SIMD target selection, multiple can be specified above
# on RISC targets top value above is chosen by default, use -n/-k/-s to override
# 256-bit SIMD is achieved by combining pairs of 128-bit registers/instructions
# 512-bit SIMD is achieved by combining quads of 128-bit registers/instructions
# For 30 256-bit VSX2/3 registers on POWER8/9 targets use (replace): RT_256=4+8
# For 15 512-bit VSX2/3 registers on POWER8/9 targets use (replace): RT_512=1+2
# SIMD-buffers only work up to RT_256=1+2 due to instruction limitation (no 512)

# For interpretation of SIMD build flags check compatibility layer in rtzero.h
# or refer to the corresponding simd_make_***.mk file.

# 64/32-bit (ptr/adr) hybrid mode is compatible with native 64-bit ABI,
# use (replace): RT_ADDRESS=32, rename the binary to core_test.p64_**
# 64-bit packed SIMD mode (fp64/int64) is supported on 64-bit targets,
# use (replace): RT_ELEMENT=64, rename the binary to core_test.p64*64
