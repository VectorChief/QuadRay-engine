
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


build: core_test_x86

strip:
	strip core_test.x86

clean:
	rm core_test.x86


core_test_x86:
	g++ -O3 -g -static -m32 \
        -DRT_LINUX -DRT_X86 -DRT_128=1+2+4+8 -DRT_256=1+2 \
        -DRT_POINTER=32 -DRT_ADDRESS=32 -DRT_ELEMENT=32 -DRT_ENDIAN=0 \
        -DRT_DEBUG=0 -DRT_PATH="../" \
        -DRT_EMBED_STDOUT=0 -DRT_EMBED_FILEIO=0 -DRT_EMBED_TEX=1 \
        ${INC_PATH} ${SRC_LIST} ${LIB_PATH} ${LIB_LIST} -o core_test.x86


# Prerequisites for the build:
# native/multilib-compiler for x86/x86_64 is installed and in the PATH variable.
# sudo apt-get install g++ (for x86 host)
# sudo apt-get install g++-multilib (for x86_64 host)
# (installation of g++-multilib removes any g++ cross-compilers)
#
# Building/running CORE test:
# make -f core_make_x86.mk
# ./core_test.x86 -i -a
# (should produce antialiased (-a) images (-i) in the ../dump subfolder)

# core_test uses runtime SIMD target selection, multiple can be specified above

# Clang compilation works too (takes much longer prior to 3.8), use (replace):
# clang++ -Wno-logical-op-parentheses -Wno-bitwise-op-parentheses
# sudo apt-get install clang (requires g++-multilib for non-native ABI)
