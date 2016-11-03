
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
        ../core/tracer/tracer_128v2.cpp \
        ../core/tracer/tracer_128v4.cpp \
        ../core/tracer/tracer_128v8.cpp \
        ../core/tracer/tracer_256v1.cpp \
        ../core/tracer/tracer_256v2.cpp \
        core_test.cpp

LIB_PATH =

LIB_LIST =                              \
        -lm                             \
        -lstdc++


build: core_test_x64_32 core_test_x64_64 core_test_x64f32 core_test_x64f64

strip:
	x86_64-linux-gnu-strip core_test.x64*

clean:
	rm core_test.x64*


core_test_x64_32:
	x86_64-linux-gnu-g++ -O3 -g -static \
        -DRT_LINUX -DRT_X64 -DRT_128=2+4+8 -DRT_256=1+2 \
        -DRT_POINTER=64 -DRT_ADDRESS=32 -DRT_ELEMENT=32 -DRT_ENDIAN=0 \
        -DRT_DEBUG=0 -DRT_PATH="../" \
        -DRT_EMBED_STDOUT=0 -DRT_EMBED_FILEIO=0 -DRT_EMBED_TEX=1 \
        ${INC_PATH} ${SRC_LIST} ${LIB_PATH} ${LIB_LIST} -o core_test.x64_32

core_test_x64_64:
	x86_64-linux-gnu-g++ -O3 -g -static \
        -DRT_LINUX -DRT_X64 -DRT_128=2+4+8 -DRT_256=1+2 \
        -DRT_POINTER=64 -DRT_ADDRESS=32 -DRT_ELEMENT=64 -DRT_ENDIAN=0 \
        -DRT_DEBUG=0 -DRT_PATH="../" \
        -DRT_EMBED_STDOUT=0 -DRT_EMBED_FILEIO=0 -DRT_EMBED_TEX=1 \
        ${INC_PATH} ${SRC_LIST} ${LIB_PATH} ${LIB_LIST} -o core_test.x64_64

core_test_x64f32:
	x86_64-linux-gnu-g++ -O3 -g -static \
        -DRT_LINUX -DRT_X64 -DRT_128=2+4+8 -DRT_256=1+2 \
        -DRT_POINTER=64 -DRT_ADDRESS=64 -DRT_ELEMENT=32 -DRT_ENDIAN=0 \
        -DRT_DEBUG=0 -DRT_PATH="../" \
        -DRT_EMBED_STDOUT=0 -DRT_EMBED_FILEIO=0 -DRT_EMBED_TEX=1 \
        ${INC_PATH} ${SRC_LIST} ${LIB_PATH} ${LIB_LIST} -o core_test.x64f32

core_test_x64f64:
	x86_64-linux-gnu-g++ -O3 -g -static \
        -DRT_LINUX -DRT_X64 -DRT_128=2+4+8 -DRT_256=1+2 \
        -DRT_POINTER=64 -DRT_ADDRESS=64 -DRT_ELEMENT=64 -DRT_ENDIAN=0 \
        -DRT_DEBUG=0 -DRT_PATH="../" \
        -DRT_EMBED_STDOUT=0 -DRT_EMBED_FILEIO=0 -DRT_EMBED_TEX=1 \
        ${INC_PATH} ${SRC_LIST} ${LIB_PATH} ${LIB_LIST} -o core_test.x64f64


# Prerequisites for the build:
# native-compiler for x86_64 is installed and in the PATH variable.
# sudo apt-get install g++
#
# Building/running CORE test:
# make -f core_make_x64.mk
# ./core_test.x64f32 -i -a
# (should produce antialiased (-a) images (-i) in the ../dump subfolder)

# core_test uses runtime SIMD target selection, multiple can be specified above

# Clang compilation works too (takes much longer prior to 3.8), use (replace):
# clang++ -Wno-logical-op-parentheses -Wno-bitwise-op-parentheses
# sudo apt-get install clang

# 64/32-bit (ptr/adr) hybrid mode is compatible with native 64-bit ABI,
# use (replace): RT_ADDRESS=32, rename the binary to core_test.x64_**

# 64-bit packed SIMD mode (fp64/int64) is supported on 64-bit targets,
# use (replace): RT_ELEMENT=64, rename the binary to core_test.x64*64
