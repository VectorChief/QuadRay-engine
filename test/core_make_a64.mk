
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
        ../core/tracer/tracer_128v4.cpp     \
        ../core/tracer/tracer_256v1.cpp     \
        ../core/tracer/tracer_256v4.cpp     \
        ../core/tracer/tracer_512v1.cpp     \
        ../core/tracer/tracer_512v4.cpp     \
        ../core/tracer/tracer_1K4v1.cpp     \
        ../core/tracer/tracer_1K4v4.cpp     \
        ../core/tracer/tracer_2K8v1_r8.cpp  \
        ../core/tracer/tracer_2K8v4_r8.cpp  \
        core_test.cpp

LIB_PATH =

LIB_LIST =                                  \
        -lm                                 \
        -lstdc++


build: build_a64 build_a64sve

strip:
	aarch64-linux-gnu-strip core_test.a64*

clean:
	rm core_test.a64*


build_a64: core_test_a64_32 core_test_a64_64 core_test_a64f32 core_test_a64f64

core_test_a64_32:
	aarch64-linux-gnu-g++ -O3 -g -static \
        -DRT_LINUX -DRT_A64 -DRT_128=1 -DRT_256=1 \
        -DRT_POINTER=64 -DRT_ADDRESS=32 -DRT_ELEMENT=32 -DRT_ENDIAN=0 \
        -DRT_DEBUG=0 -DRT_PATH="../" \
        -DRT_EMBED_STDOUT=0 -DRT_EMBED_FILEIO=0 -DRT_EMBED_TEX=1 \
        ${INC_PATH} ${SRC_LIST} ${LIB_PATH} ${LIB_LIST} -o core_test.a64_32

core_test_a64_64:
	aarch64-linux-gnu-g++ -O3 -g -static \
        -DRT_LINUX -DRT_A64 -DRT_128=1 -DRT_256=1 \
        -DRT_POINTER=64 -DRT_ADDRESS=32 -DRT_ELEMENT=64 -DRT_ENDIAN=0 \
        -DRT_DEBUG=0 -DRT_PATH="../" \
        -DRT_EMBED_STDOUT=0 -DRT_EMBED_FILEIO=0 -DRT_EMBED_TEX=1 \
        ${INC_PATH} ${SRC_LIST} ${LIB_PATH} ${LIB_LIST} -o core_test.a64_64

core_test_a64f32:
	aarch64-linux-gnu-g++ -O3 -g -static \
        -DRT_LINUX -DRT_A64 -DRT_128=1 -DRT_256=1 \
        -DRT_POINTER=64 -DRT_ADDRESS=64 -DRT_ELEMENT=32 -DRT_ENDIAN=0 \
        -DRT_DEBUG=0 -DRT_PATH="../" \
        -DRT_EMBED_STDOUT=0 -DRT_EMBED_FILEIO=0 -DRT_EMBED_TEX=1 \
        ${INC_PATH} ${SRC_LIST} ${LIB_PATH} ${LIB_LIST} -o core_test.a64f32

core_test_a64f64:
	aarch64-linux-gnu-g++ -O3 -g -static \
        -DRT_LINUX -DRT_A64 -DRT_128=1 -DRT_256=1 \
        -DRT_POINTER=64 -DRT_ADDRESS=64 -DRT_ELEMENT=64 -DRT_ENDIAN=0 \
        -DRT_DEBUG=0 -DRT_PATH="../" \
        -DRT_EMBED_STDOUT=0 -DRT_EMBED_FILEIO=0 -DRT_EMBED_TEX=1 \
        ${INC_PATH} ${SRC_LIST} ${LIB_PATH} ${LIB_LIST} -o core_test.a64f64


build_a64sve: core_test_a64_32sve core_test_a64_64sve \
              core_test_a64f32sve core_test_a64f64sve

core_test_a64_32sve:
	aarch64-linux-gnu-g++ -O3 -g -static \
        -DRT_LINUX -DRT_A64 -DRT_128=1 -DRT_256=1+4 -DRT_512=1+4 -DRT_1K4=1+4 \
        -DRT_POINTER=64 -DRT_ADDRESS=32 -DRT_ELEMENT=32 -DRT_ENDIAN=0 \
        -DRT_DEBUG=0 -DRT_PATH="../" \
        -DRT_EMBED_STDOUT=0 -DRT_EMBED_FILEIO=0 -DRT_EMBED_TEX=1 \
        ${INC_PATH} ${SRC_LIST} ${LIB_PATH} ${LIB_LIST} -o core_test.a64_32sve

core_test_a64_64sve:
	aarch64-linux-gnu-g++ -O3 -g -static \
        -DRT_LINUX -DRT_A64 -DRT_128=1 -DRT_256=1+4 -DRT_512=1+4 -DRT_1K4=1+4 \
        -DRT_POINTER=64 -DRT_ADDRESS=32 -DRT_ELEMENT=64 -DRT_ENDIAN=0 \
        -DRT_DEBUG=0 -DRT_PATH="../" \
        -DRT_EMBED_STDOUT=0 -DRT_EMBED_FILEIO=0 -DRT_EMBED_TEX=1 \
        ${INC_PATH} ${SRC_LIST} ${LIB_PATH} ${LIB_LIST} -o core_test.a64_64sve

core_test_a64f32sve:
	aarch64-linux-gnu-g++ -O3 -g -static \
        -DRT_LINUX -DRT_A64 -DRT_128=1 -DRT_256=1+4 -DRT_512=1+4 -DRT_1K4=1+4 \
        -DRT_POINTER=64 -DRT_ADDRESS=64 -DRT_ELEMENT=32 -DRT_ENDIAN=0 \
        -DRT_DEBUG=0 -DRT_PATH="../" \
        -DRT_EMBED_STDOUT=0 -DRT_EMBED_FILEIO=0 -DRT_EMBED_TEX=1 \
        ${INC_PATH} ${SRC_LIST} ${LIB_PATH} ${LIB_LIST} -o core_test.a64f32sve

core_test_a64f64sve:
	aarch64-linux-gnu-g++ -O3 -g -static \
        -DRT_LINUX -DRT_A64 -DRT_128=1 -DRT_256=1+4 -DRT_512=1+4 -DRT_1K4=1+4 \
        -DRT_POINTER=64 -DRT_ADDRESS=64 -DRT_ELEMENT=64 -DRT_ENDIAN=0 \
        -DRT_DEBUG=0 -DRT_PATH="../" \
        -DRT_EMBED_STDOUT=0 -DRT_EMBED_FILEIO=0 -DRT_EMBED_TEX=1 \
        ${INC_PATH} ${SRC_LIST} ${LIB_PATH} ${LIB_LIST} -o core_test.a64f64sve


# On Ubuntu (Mate) 16.04/18.04 add "universe multiverse" to "main restricted"
# in /etc/apt/sources.list (sudo nano /etc/apt/sources.list) then run:
# sudo apt-get update
#
# Prerequisites for the build:
# (cross-)compiler for AArch64 is installed and in the PATH variable.
# sudo apt-get install make g++-aarch64-linux-gnu
#
# Prerequisites for emulation:
# recent QEMU(-2.5) is installed or built from source and in the PATH variable.
# SVE targets require QEMU 3.1.0 (or 3.0.0 with sve-max-vq cpu property patch).
# sudo apt-get install qemu-user
#
# Building/running CORE test:
# make -f core_make_a64.mk
# qemu-aarch64 -cpu cortex-a57 core_test.a64f32 -i -a -c 1
# qemu-aarch64 -cpu max,sve-max-vq=2 core_test.a64f32sve -i -a -c 1
# qemu-aarch64 -cpu max,sve-max-vq=4 core_test.a64f32sve -i -a -c 1
# qemu-aarch64 -cpu max,sve-max-vq=8 core_test.a64f32sve -i -a -c 1
# (should produce antialiased "-a" images "-i" in the ../dump subfolder)
# Use "-c 1" option to reduce test time when emulating with QEMU

# Clang native build works too (takes much longer prior to 3.8), use (replace):
# clang++ (in place of ...-g++) on AArch64 host (Raspberry Pi 3)
# sudo apt-get install clang

# core_test uses runtime SIMD target selection, multiple can be specified above
# on RISC targets top value above is chosen by default, use -n/-k/-s to override
# 256-bit SIMD is achieved by combining pairs of 128-bit registers/instructions
# For 2048-bit SVE build use (replace): RT_2K8_R8=1+4

# For interpretation of SIMD build flags check compatibility layer in rtzero.h
# or refer to the corresponding simd_make_***.mk file.

# 64/32-bit (ptr/adr) hybrid mode is compatible with native 64-bit ABI,
# use (replace): RT_ADDRESS=32, rename the binary to core_test.a64_**
# 64-bit packed SIMD mode (fp64/int64) is supported on 64-bit targets,
# use (replace): RT_ELEMENT=64, rename the binary to core_test.a64*64
