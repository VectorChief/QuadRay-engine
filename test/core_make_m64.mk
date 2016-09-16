
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


build: build_le build_be

strip:
	mips-img-linux-gnu-strip core_test.m64*

clean:
	rm core_test.m64*


build_le: core_test_m64_32Lr6 core_test_m64f32Lr6 core_test_m64f64Lr6

core_test_m64_32Lr6:
	mips-img-linux-gnu-g++ -O3 -g -static -EL -mabi=64 -mmsa \
        -DRT_LINUX -DRT_M64=6 -DRT_128=1 \
        -DRT_POINTER=64 -DRT_ADDRESS=32 -DRT_ELEMENT=32 -DRT_ENDIAN=0 \
        -DRT_DEBUG=0 -DRT_PATH="../" \
        -DRT_EMBED_STDOUT=0 -DRT_EMBED_FILEIO=0 -DRT_EMBED_TEX=1 \
        ${INC_PATH} ${SRC_LIST} ${LIB_PATH} ${LIB_LIST} -o core_test.m64_32Lr6

core_test_m64f32Lr6:
	mips-img-linux-gnu-g++ -O3 -g -static -EL -mabi=64 -mmsa \
        -DRT_LINUX -DRT_M64=6 -DRT_128=1 \
        -DRT_POINTER=64 -DRT_ADDRESS=64 -DRT_ELEMENT=32 -DRT_ENDIAN=0 \
        -DRT_DEBUG=0 -DRT_PATH="../" \
        -DRT_EMBED_STDOUT=0 -DRT_EMBED_FILEIO=0 -DRT_EMBED_TEX=1 \
        ${INC_PATH} ${SRC_LIST} ${LIB_PATH} ${LIB_LIST} -o core_test.m64f32Lr6

core_test_m64f64Lr6:
	mips-img-linux-gnu-g++ -O3 -g -static -EL -mabi=64 -mmsa \
        -DRT_LINUX -DRT_M64=6 -DRT_128=1 \
        -DRT_POINTER=64 -DRT_ADDRESS=64 -DRT_ELEMENT=64 -DRT_ENDIAN=0 \
        -DRT_DEBUG=0 -DRT_PATH="../" \
        -DRT_EMBED_STDOUT=0 -DRT_EMBED_FILEIO=0 -DRT_EMBED_TEX=1 \
        ${INC_PATH} ${SRC_LIST} ${LIB_PATH} ${LIB_LIST} -o core_test.m64f64Lr6


build_be: core_test_m64_32Br6 core_test_m64f32Br6 core_test_m64f64Br6

core_test_m64_32Br6:
	mips-img-linux-gnu-g++ -O3 -g -static -EB -mabi=64 -mmsa \
        -DRT_LINUX -DRT_M64=6 -DRT_128=1 \
        -DRT_POINTER=64 -DRT_ADDRESS=32 -DRT_ELEMENT=32 -DRT_ENDIAN=1 \
        -DRT_DEBUG=0 -DRT_PATH="../" \
        -DRT_EMBED_STDOUT=0 -DRT_EMBED_FILEIO=0 -DRT_EMBED_TEX=1 \
        ${INC_PATH} ${SRC_LIST} ${LIB_PATH} ${LIB_LIST} -o core_test.m64_32Br6

core_test_m64f32Br6:
	mips-img-linux-gnu-g++ -O3 -g -static -EB -mabi=64 -mmsa \
        -DRT_LINUX -DRT_M64=6 -DRT_128=1 \
        -DRT_POINTER=64 -DRT_ADDRESS=64 -DRT_ELEMENT=32 -DRT_ENDIAN=1 \
        -DRT_DEBUG=0 -DRT_PATH="../" \
        -DRT_EMBED_STDOUT=0 -DRT_EMBED_FILEIO=0 -DRT_EMBED_TEX=1 \
        ${INC_PATH} ${SRC_LIST} ${LIB_PATH} ${LIB_LIST} -o core_test.m64f32Br6

core_test_m64f64Br6:
	mips-img-linux-gnu-g++ -O3 -g -static -EB -mabi=64 -mmsa \
        -DRT_LINUX -DRT_M64=6 -DRT_128=1 \
        -DRT_POINTER=64 -DRT_ADDRESS=64 -DRT_ELEMENT=64 -DRT_ENDIAN=1 \
        -DRT_DEBUG=0 -DRT_PATH="../" \
        -DRT_EMBED_STDOUT=0 -DRT_EMBED_FILEIO=0 -DRT_EMBED_TEX=1 \
        ${INC_PATH} ${SRC_LIST} ${LIB_PATH} ${LIB_LIST} -o core_test.m64f64Br6


# The up-to-date MIPS toolchain (g++ & QEMU) can be found here:
# https://community.imgtec.com/developers/mips/tools/codescape-mips-sdk/

# On Ubuntu 16.04 Live CD add "universe multiverse" to "main restricted"
# in /etc/apt/sources.list (sudo gedit /etc/apt/sources.list) then run:
# sudo apt-get update (ignoring the old database errors in the end)
#
# Prerequisites for the build:
# (cross-)compiler for MIPSr6+MSA is installed and in the PATH variable.
# Codescape.GNU.Tools.Package.2016.05-03.for.MIPS.IMG.Linux.CentOS-5.x86_64
# is unpacked and folder mips-img-linux-gnu/2016.05-03/bin is added to PATH:
# PATH=/home/ubuntu/Downloads/mips-img-linux-gnu/2016.05-03/bin:$PATH
#
# Prerequisites for emulation:
# QEMU 2.5.0.2.0 from imgtec.com is built from source and in the PATH variable.
# Unpack qemu-rel-2.5.0.2.0 archive and change to its root folder, then run:
# sudo apt-get install zlib1g-dev libglib2.0-dev libpixman-1-dev
# ./configure --target-list=mips64el-linux-user,mips64-linux-user
# make -j8
# sudo make install
# (building from source makes QEMU's mmap forwarding honour address hint)
#
# Building/running CORE test:
# make -f core_make_m64.mk
# qemu-mips64el -cpu I6400 core_test.m64f32Lr6 -i -a
# (should produce antialiased (-a) images (-i) in the ../dump subfolder)

# For big-endian MIPS64 target use (replace): -EB -DRT_ENDIAN=1
# qemu-mips64 -cpu I6400 core_test.m64f32Br6 -i -a
# (should produce antialiased (-a) images (-i) in the ../dump subfolder)

# 64/32-bit (ptr/adr) hybrid mode compatible with native 64-bit ABI
# is available for the original pure 32-bit ISA using 64-bit pointers,
# use (replace): RT_ADDRESS=32, rename the binary to core_test.m64_32

# 64-bit packed SIMD mode (fp64/int64) is supported on 64-bit targets,
# but currently requires addresses to be 64-bit as well (RT_ADDRESS=64),
# use (replace): RT_ELEMENT=64, rename the binary to core_test.m64f64
