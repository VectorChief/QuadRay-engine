
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


build: build_le build_be

strip:
	mips-mti-linux-gnu-strip core_test.m64???Lr6
	mips-mti-linux-gnu-strip core_test.m64???Br6

clean:
	rm core_test.m64*


build_le: core_test_m64_32Lr6 core_test_m64_64Lr6 \
          core_test_m64f32Lr6 core_test_m64f64Lr6

core_test_m64_32Lr6:
	mips-mti-linux-gnu-g++ -O3 -g -static -EL -mips64r6 -mmsa -mabi=64 \
        -DRT_LINUX -DRT_M64=6 -DRT_128=1 -DRT_256=1 \
        -DRT_POINTER=64 -DRT_ADDRESS=32 -DRT_ELEMENT=32 -DRT_ENDIAN=0 \
        -DRT_DEBUG=0 -DRT_PATH="../" \
        -DRT_EMBED_STDOUT=0 -DRT_EMBED_FILEIO=0 -DRT_EMBED_TEX=1 \
        ${INC_PATH} ${SRC_LIST} ${LIB_PATH} ${LIB_LIST} -o core_test.m64_32Lr6

core_test_m64_64Lr6:
	mips-mti-linux-gnu-g++ -O3 -g -static -EL -mips64r6 -mmsa -mabi=64 \
        -DRT_LINUX -DRT_M64=6 -DRT_128=1 -DRT_256=1 \
        -DRT_POINTER=64 -DRT_ADDRESS=32 -DRT_ELEMENT=64 -DRT_ENDIAN=0 \
        -DRT_DEBUG=0 -DRT_PATH="../" \
        -DRT_EMBED_STDOUT=0 -DRT_EMBED_FILEIO=0 -DRT_EMBED_TEX=1 \
        ${INC_PATH} ${SRC_LIST} ${LIB_PATH} ${LIB_LIST} -o core_test.m64_64Lr6

core_test_m64f32Lr6:
	mips-mti-linux-gnu-g++ -O3 -g -static -EL -mips64r6 -mmsa -mabi=64 \
        -DRT_LINUX -DRT_M64=6 -DRT_128=1 -DRT_256=1 \
        -DRT_POINTER=64 -DRT_ADDRESS=64 -DRT_ELEMENT=32 -DRT_ENDIAN=0 \
        -DRT_DEBUG=0 -DRT_PATH="../" \
        -DRT_EMBED_STDOUT=0 -DRT_EMBED_FILEIO=0 -DRT_EMBED_TEX=1 \
        ${INC_PATH} ${SRC_LIST} ${LIB_PATH} ${LIB_LIST} -o core_test.m64f32Lr6

core_test_m64f64Lr6:
	mips-mti-linux-gnu-g++ -O3 -g -static -EL -mips64r6 -mmsa -mabi=64 \
        -DRT_LINUX -DRT_M64=6 -DRT_128=1 -DRT_256=1 \
        -DRT_POINTER=64 -DRT_ADDRESS=64 -DRT_ELEMENT=64 -DRT_ENDIAN=0 \
        -DRT_DEBUG=0 -DRT_PATH="../" \
        -DRT_EMBED_STDOUT=0 -DRT_EMBED_FILEIO=0 -DRT_EMBED_TEX=1 \
        ${INC_PATH} ${SRC_LIST} ${LIB_PATH} ${LIB_LIST} -o core_test.m64f64Lr6


build_be: core_test_m64_32Br6 core_test_m64_64Br6 \
          core_test_m64f32Br6 core_test_m64f64Br6

core_test_m64_32Br6:
	mips-mti-linux-gnu-g++ -O3 -g -static -EB -mips64r6 -mmsa -mabi=64 \
        -DRT_LINUX -DRT_M64=6 -DRT_128=1 -DRT_256=1 \
        -DRT_POINTER=64 -DRT_ADDRESS=32 -DRT_ELEMENT=32 -DRT_ENDIAN=1 \
        -DRT_DEBUG=0 -DRT_PATH="../" \
        -DRT_EMBED_STDOUT=0 -DRT_EMBED_FILEIO=0 -DRT_EMBED_TEX=1 \
        ${INC_PATH} ${SRC_LIST} ${LIB_PATH} ${LIB_LIST} -o core_test.m64_32Br6

core_test_m64_64Br6:
	mips-mti-linux-gnu-g++ -O3 -g -static -EB -mips64r6 -mmsa -mabi=64 \
        -DRT_LINUX -DRT_M64=6 -DRT_128=1 -DRT_256=1 \
        -DRT_POINTER=64 -DRT_ADDRESS=32 -DRT_ELEMENT=64 -DRT_ENDIAN=1 \
        -DRT_DEBUG=0 -DRT_PATH="../" \
        -DRT_EMBED_STDOUT=0 -DRT_EMBED_FILEIO=0 -DRT_EMBED_TEX=1 \
        ${INC_PATH} ${SRC_LIST} ${LIB_PATH} ${LIB_LIST} -o core_test.m64_64Br6

core_test_m64f32Br6:
	mips-mti-linux-gnu-g++ -O3 -g -static -EB -mips64r6 -mmsa -mabi=64 \
        -DRT_LINUX -DRT_M64=6 -DRT_128=1 -DRT_256=1 \
        -DRT_POINTER=64 -DRT_ADDRESS=64 -DRT_ELEMENT=32 -DRT_ENDIAN=1 \
        -DRT_DEBUG=0 -DRT_PATH="../" \
        -DRT_EMBED_STDOUT=0 -DRT_EMBED_FILEIO=0 -DRT_EMBED_TEX=1 \
        ${INC_PATH} ${SRC_LIST} ${LIB_PATH} ${LIB_LIST} -o core_test.m64f32Br6

core_test_m64f64Br6:
	mips-mti-linux-gnu-g++ -O3 -g -static -EB -mips64r6 -mmsa -mabi=64 \
        -DRT_LINUX -DRT_M64=6 -DRT_128=1 -DRT_256=1 \
        -DRT_POINTER=64 -DRT_ADDRESS=64 -DRT_ELEMENT=64 -DRT_ENDIAN=1 \
        -DRT_DEBUG=0 -DRT_PATH="../" \
        -DRT_EMBED_STDOUT=0 -DRT_EMBED_FILEIO=0 -DRT_EMBED_TEX=1 \
        ${INC_PATH} ${SRC_LIST} ${LIB_PATH} ${LIB_LIST} -o core_test.m64f64Br6


# On Ubuntu (MATE) 16.04/18.04 download MIPS toolchain:
# https://www.mips.com/develop/tools/codescape-mips-sdk/
# https://codescape.mips.com/components/toolchain/2020.06-01/downloads.html
#
# Prerequisites for the build:
# (cross-)compiler for MIPSr6+MSA is installed and in the PATH variable.
# Codescape.GNU.Tools.Package.2020.06-01.for.MIPS.MTI.Linux.CentOS-6.x86_64
# is unpacked and folder mips-mti-linux-gnu/2020.06-01/bin is added to PATH:
# PATH=/home/ubuntu/Downloads/mips-mti-linux-gnu/2020.06-01/bin:$PATH
# PATH=/home/ubuntu-mate/Downloads/mips-mti-linux-gnu/2020.06-01/bin:$PATH
#
# Starting from Ubuntu (MATE) 19.10 upstream (cross-)compiler supports MSA.
# sudo apt-get install make g++-mipsisa64r6el-linux-gnuabi64
# sudo apt-get install make g++-mipsisa64r6-linux-gnuabi64
# (replace mips-mti-linux-gnu-g++ with mipsisa64r6el-linux-gnuabi64-g++ for LE)
# (replace mips-mti-linux-gnu-g++ with mipsisa64r6-linux-gnuabi64-g++ for BE)
#
# Prerequisites for emulation:
# recent QEMU(-2.7) is installed or built from source and in the PATH variable.
# standalone toolchain from 2020.06-01 comes with QEMU 4.1.0 for MIPS in PATH.
# sudo apt-get install qemu-user make
#
# Compiling/running CORE test:
# make -f core_make_m64.mk
# qemu-mips64el -cpu I6400 core_test.m64f32Lr6 -i -a -c 1
# qemu-mips64   -cpu I6400 core_test.m64f32Br6 -i -a -c 1
# (should produce antialiased "-a" images "-i" in the ../dump subfolder)
# Use "-c 1" option to reduce test time when emulating with QEMU

# Clang native build should theoretically work too (not tested), use (replace):
# clang++ (in place of ...-g++) on MIPS64 host
# sudo apt-get install clang

# core_test uses runtime SIMD target selection, multiple can be specified above
# on RISC targets top value above is chosen by default, use -n/-k/-s to override
# 256-bit SIMD is achieved by combining pairs of 128-bit registers/instructions

# For interpretation of SIMD build flags check compatibility layer in rtzero.h
# or refer to the corresponding simd_make_***.mk file.

# 64/32-bit (ptr/adr) hybrid mode is compatible with native 64-bit ABI,
# use (replace): RT_ADDRESS=32, rename the binary to core_test.m64_**
# 64-bit packed SIMD mode (fp64/int64) is supported on 64-bit targets,
# use (replace): RT_ELEMENT=64, rename the binary to core_test.m64*64
