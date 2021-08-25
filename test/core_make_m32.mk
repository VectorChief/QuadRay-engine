
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
        -lm                                 \
        -lstdc++


build: core_test_m32Lr5 core_test_m32Br5

strip:
	mips-mti-linux-gnu-strip core_test.m32?r5*

clean:
	rm core_test.m32*


core_test_m32Lr5:
	mips-mti-linux-gnu-g++ -O3 -g -static -EL -mips32r5 -mmsa -mnan=2008 \
        -DRT_LINUX -DRT_M32 -DRT_128=1 -DRT_256=1 \
        -DRT_POINTER=32 -DRT_ADDRESS=32 -DRT_ELEMENT=32 -DRT_ENDIAN=0 \
        -DRT_DEBUG=0 -DRT_PATH="../" \
        -DRT_EMBED_STDOUT=0 -DRT_EMBED_FILEIO=0 -DRT_EMBED_TEX=1 \
        ${INC_PATH} ${SRC_LIST} ${LIB_PATH} ${LIB_LIST} -o core_test.m32Lr5

core_test_m32Br5:
	mips-mti-linux-gnu-g++ -O3 -g -static -EB -mips32r5 -mmsa -mnan=2008 \
        -DRT_LINUX -DRT_M32 -DRT_128=1 -DRT_256=1 \
        -DRT_POINTER=32 -DRT_ADDRESS=32 -DRT_ELEMENT=32 -DRT_ENDIAN=1 \
        -DRT_DEBUG=0 -DRT_PATH="../" \
        -DRT_EMBED_STDOUT=0 -DRT_EMBED_FILEIO=0 -DRT_EMBED_TEX=1 \
        ${INC_PATH} ${SRC_LIST} ${LIB_PATH} ${LIB_LIST} -o core_test.m32Br5


# On Ubuntu (MATE) 16.04-20.04 add "universe multiverse" to "main restricted"
# in /etc/apt/sources.list (sudo nano /etc/apt/sources.list) then run:
# sudo apt-get update
# (Ubuntu MATE is set up for an update without a need to edit the file)
# (extended repositories "universe multiverse" are only needed for clang)
#
# Download and unpack MIPS toolchain:
# https://www.mips.com/develop/tools/codescape-mips-sdk/
# https://codescape.mips.com/components/toolchain/2020.06-01/downloads.html
#
# Prerequisites for the build:
# (cross-)compiler for MIPSr5+MSA is installed and in the PATH variable.
# Codescape.GNU.Tools.Package.2020.06-01.for.MIPS.MTI.Linux.CentOS-6.x86_64
# is unpacked and folder mips-mti-linux-gnu/2020.06-01/bin is added to PATH:
# PATH=/home/ubuntu/Downloads/mips-mti-linux-gnu/2020.06-01/bin:$PATH
# PATH=/home/ubuntu-mate/Downloads/mips-mti-linux-gnu/2020.06-01/bin:$PATH
#
# Prerequisites for emulation:
# recent QEMU(-2.5) is installed or built from source and in the PATH variable.
# standalone toolchain from 2020.06-01 comes with QEMU 4.1.0 for MIPS in PATH.
# sudo apt-get install qemu-user make
#
# Compiling/running CORE test:
# make -f core_make_m32.mk
# qemu-mipsel -cpu P5600 core_test.m32Lr5 -i -a -c 1
# qemu-mips   -cpu P5600 core_test.m32Br5 -i -a -c 1
# (should produce antialiased "-a" images "-i" in the ../dump subfolder)
# Use "-c 1" option to reduce test time when emulating with QEMU

# Clang native build should theoretically work too (not tested), use (replace):
# clang++ -O0 (in place of ...-g++ -O3) on MIPS32r5 host (P5600)
# sudo apt-get install clang

# core_test uses runtime SIMD target selection, multiple can be specified above
# on RISC targets top value above is chosen by default, use -n/-k/-s to override
# 256-bit SIMD is achieved by combining pairs of 128-bit registers/instructions

# For interpretation of SIMD build flags check compatibility layer in rtzero.h
# or refer to the corresponding simd_make_***.mk file.
