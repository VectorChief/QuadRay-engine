
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
        core_test.cpp

LIB_PATH =

LIB_LIST =                                  \
        -lm


build: build_z64

strip:
	s390x-linux-gnu-strip core_test.z64*

clean:
	rm core_test.z64*


build_z64: core_test_z64_32 core_test_z64_64 \
           core_test_z64f32 core_test_z64f64

core_test_z64_32:
	s390x-linux-gnu-g++ -O3 -g -static \
        -DRT_LINUX -DRT_Z64 -DRT_128=1 -DRT_256=1 \
        -DRT_POINTER=64 -DRT_ADDRESS=32 -DRT_ELEMENT=32 -DRT_ENDIAN=1 \
        -DRT_DEBUG=0 -DRT_PATH="../" \
        -DRT_EMBED_STDOUT=0 -DRT_EMBED_FILEIO=0 -DRT_EMBED_TEX=1 \
        ${INC_PATH} ${SRC_LIST} ${LIB_PATH} ${LIB_LIST} -o core_test.z64_32

core_test_z64_64:
	s390x-linux-gnu-g++ -O3 -g -static \
        -DRT_LINUX -DRT_Z64 -DRT_128=1 -DRT_256=1 \
        -DRT_POINTER=64 -DRT_ADDRESS=32 -DRT_ELEMENT=64 -DRT_ENDIAN=1 \
        -DRT_DEBUG=0 -DRT_PATH="../" \
        -DRT_EMBED_STDOUT=0 -DRT_EMBED_FILEIO=0 -DRT_EMBED_TEX=1 \
        ${INC_PATH} ${SRC_LIST} ${LIB_PATH} ${LIB_LIST} -o core_test.z64_64

core_test_z64f32:
	s390x-linux-gnu-g++ -O3 -g -static \
        -DRT_LINUX -DRT_Z64 -DRT_128=1 -DRT_256=1 \
        -DRT_POINTER=64 -DRT_ADDRESS=64 -DRT_ELEMENT=32 -DRT_ENDIAN=1 \
        -DRT_DEBUG=0 -DRT_PATH="../" \
        -DRT_EMBED_STDOUT=0 -DRT_EMBED_FILEIO=0 -DRT_EMBED_TEX=1 \
        ${INC_PATH} ${SRC_LIST} ${LIB_PATH} ${LIB_LIST} -o core_test.z64f32

core_test_z64f64:
	s390x-linux-gnu-g++ -O3 -g -static \
        -DRT_LINUX -DRT_Z64 -DRT_128=1 -DRT_256=1 \
        -DRT_POINTER=64 -DRT_ADDRESS=64 -DRT_ELEMENT=64 -DRT_ENDIAN=1 \
        -DRT_DEBUG=0 -DRT_PATH="../" \
        -DRT_EMBED_STDOUT=0 -DRT_EMBED_FILEIO=0 -DRT_EMBED_TEX=1 \
        ${INC_PATH} ${SRC_LIST} ${LIB_PATH} ${LIB_LIST} -o core_test.z64f64


# On Ubuntu (MATE) 16.04-26.04 add "universe multiverse" to "main restricted"
# in /etc/apt/sources.list (sudo nano /etc/apt/sources.list) then run:
# sudo apt-get update
# (Ubuntu MATE is set up for an update without a need to edit the file)
# (extended repositories "universe multiverse" are only needed for clang)
#
# Prerequisites for the build:
# (cross-)compiler for s390x is installed and in the PATH variable.
# sudo apt-get install make g++-s390x-linux-gnu
#
# Prerequisites for emulation:
# recent QEMU(-8.2) is installed or built from source and in the PATH variable.
# sudo apt-get install qemu-user
#
# Compiling/running SIMD test:
# make -f core_make_z64.mk
# qemu-s390x    -cpu max core_test.z64_32 -i -a -c 1 -k 1 (x2 backends broken)
# qemu-s390x    -cpu max core_test.z64_64 -i -a -c 1 -k 1 (drop -a -k 1 to see)
# qemu-s390x    -cpu max core_test.z64f32 -i -a -c 1 -k 1 (x2 backends broken)
# qemu-s390x    -cpu max core_test.z64f64 -i -a -c 1 -k 1 (drop -a -k 1 to see)
# (should produce antialiased "-a" images "-i" in the ../dump subfolder)
# Use "-c 1" option to reduce test time when emulating with QEMU

# Clang native build should theoretically work too (not tested), use (replace):
# clang++ -O0 (in place of ...-g++ -O3) on s390x host (z15/z17)
# sudo apt-get install clang

# For interpretation of SIMD build flags check compatibility layer in rtzero.h.
# The 128-bit 15-reg targets are supported for compatibility with x86/POWER.

# For 128-bit SIMD build use (replace): RT_128=1            (30 SIMD registers)
# For 128-bit SIMD build use (replace): RT_128=4            (15 SIMD registers)
# For 256-bit SIMD build use (replace): RT_256=1            (15 SIMD reg-pairs)

# 64/32-bit (ptr/adr) hybrid mode is compatible with native 64-bit ABI,
# use (replace): RT_ADDRESS=32, rename the binary to core_test.z64_**
# 64-bit packed SIMD mode (fp64/int64) is supported on 64-bit targets,
# use (replace): RT_ELEMENT=64, rename the binary to core_test.z64*64
