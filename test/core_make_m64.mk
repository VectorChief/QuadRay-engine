
INC_PATH =                              \
        -I../core/config/               \
        -I../core/engine/               \
        -I../core/system/               \
        -I../core/tracer/               \
        -I../data/materials/            \
        -I../data/objects/              \
        -I../data/textures/             \
        -Iscenes/

SRC_LIST =                              \
        ../core/engine/engine.cpp       \
        ../core/engine/object.cpp       \
        ../core/engine/rtgeom.cpp       \
        ../core/engine/rtimag.cpp       \
        ../core/system/system.cpp       \
        ../core/tracer/tracer.cpp       \
        ../core/tracer/tracer_128v1.cpp \
        core_test.cpp

LIB_PATH =

LIB_LIST =                              \
        -lm                             \
        -lstdc++

core_test:
	mips-img-linux-gnu-g++ -O3 -g -static -EL -mabi=64 -mmsa \
        -DRT_LINUX -DRT_M64=6 -DRT_128=1 \
        -DRT_POINTER=64 -DRT_ADDRESS=32 -DRT_ENDIAN=0 \
        -DRT_DEBUG=0 -DRT_PATH="../" \
        -DRT_EMBED_STDOUT=0 -DRT_EMBED_FILEIO=0 -DRT_EMBED_TEX=1 \
        ${INC_PATH} ${SRC_LIST} ${LIB_PATH} ${LIB_LIST} -o core_test.m64_32

# Prerequisites for the build:
# (cross-)compiler for MIPS+MSA is installed and in the PATH variable.
# Prerequisites for emulation:
# QEMU 2.4.0.1.0 from imgtec.com is built from source and in the PATH variable.
#
# make -f core_make_m64.mk
# qemu-mips64el -cpu I6400 core_test.m64_32 -i -a
# (should produce antialiased (-a) images (-i) in the ../dump subfolder)

# The up-to-date MIPS toolchain (g++ & QEMU) can be found here:
# https://community.imgtec.com/developers/mips/tools/codescape-mips-sdk/

# For MIPS64 big-endian target use (replace): -EB -DRT_ENDIAN=1
# qemu-mips64 -cpu I6400 core_test.m64_32 -i -a
# (should produce antialiased (-a) images (-i) in the ../dump subfolder)

# Experimental 64/32-bit hybrid mode is enabled by default
# until full 64-bit support is implemented in the framework.
