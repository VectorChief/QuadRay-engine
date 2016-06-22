
INC_PATH =                              \
        -I../core/config/

SRC_LIST =                              \
        simd_test.cpp

LIB_PATH =

LIB_LIST =                              \
        -lm

simd_test:
	g++ -O3 -g -static -m32 \
        -DRT_LINUX -DRT_X86 -DRT_128=2 -DRT_DEBUG=0 \
        -DRT_POINTER=32 -DRT_ADDRESS=32 -DRT_ENDIAN=0 \
        ${INC_PATH} ${SRC_LIST} ${LIB_PATH} ${LIB_LIST} -o simd_test.x86

# Prerequisites for the build:
# native/multilib-compiler for x86/x86_64 is installed and in the PATH variable.
# sudo apt-get install g++ (for x86 host)
# sudo apt-get install g++-multilib (for x86_64 host)
# (installation of g++-multilib removes any g++ cross-compilers)
#
# Building/running SIMD test:
# make -f simd_make_x86.mk
# ./simd_test.x86

# Clang compilation is supported (takes much longer prior 3.8), use (replace):
# clang++ (in place of g++)
# sudo apt-get install clang (requires g++-multilib for non-native ABI)

# For SSE1 build use (replace): RT_128=1
# For SSE4 build use (replace): RT_128=4
# For AVX1 build use (replace): RT_256=1
# For AVX2 build use (replace): RT_256=2
