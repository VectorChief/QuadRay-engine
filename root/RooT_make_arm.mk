
INC_PATH =                                  \
        -I../core/config/                   \
        -I../core/engine/                   \
        -I../core/system/                   \
        -I../core/tracer/                   \
        -I../data/materials/                \
        -I../data/objects/                  \
        -I../data/scenes/                   \
        -I../data/textures/

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
        RooT_linux.cpp

LIB_PATH =

LIB_LIST =                                  \
        -lm                                 \
        -lstdc++                            \
        -lX11                               \
        -lXext                              \
        -lpthread


build: RooT_arm_v1

strip:
	strip RooT.arm_v*

clean:
	rm RooT.arm_v*


RooT_arm_v1:
	g++ -O3 -g -pthread -no-pie -march=armv7-a -marm \
        -DRT_LINUX -DRT_ARM -DRT_128=1 \
        -DRT_POINTER=32 -DRT_ADDRESS=32 -DRT_ELEMENT=32 -DRT_ENDIAN=0 \
        -DRT_DEBUG=0 -DRT_PATH="../" -DRT_FULLSCREEN=0 \
        -DRT_EMBED_STDOUT=0 -DRT_EMBED_FILEIO=0 -DRT_EMBED_TEX=1 \
        ${INC_PATH} ${SRC_LIST} ${LIB_PATH} ${LIB_LIST} -o RooT.arm_v1


clang: RooT.arm_v1

RooT.arm_v1:
	clang++ -O3 -g -pthread -march=armv7-a -marm \
        -Wno-unknown-warning-option \
        -Wno-shift-negative-value -Wno-shift-op-parentheses \
        -Wno-logical-op-parentheses -Wno-bitwise-op-parentheses \
        -DRT_LINUX -DRT_ARM -DRT_128=1 \
        -DRT_POINTER=32 -DRT_ADDRESS=32 -DRT_ELEMENT=32 -DRT_ENDIAN=0 \
        -DRT_DEBUG=0 -DRT_PATH="../" -DRT_FULLSCREEN=0 \
        -DRT_EMBED_STDOUT=0 -DRT_EMBED_FILEIO=0 -DRT_EMBED_TEX=1 \
        ${INC_PATH} ${SRC_LIST} ${LIB_PATH} ${LIB_LIST} -o RooT.arm_v1


build_n900: RooT_arm_n900

strip_n900:
	strip RooT.arm_n900*

clean_n900:
	rm RooT.arm_n900*


RooT_arm_n900:
	g++ -O3 -g -pthread -march=armv7-a -marm \
        -DRT_LINUX -DRT_ARM -DRT_128=1 \
        -DRT_POINTER=32 -DRT_ADDRESS=32 -DRT_ELEMENT=32 -DRT_ENDIAN=0 \
        -DRT_DEBUG=0 -DRT_PATH="../" -DRT_FULLSCREEN=1 \
        -DRT_EMBED_STDOUT=0 -DRT_EMBED_FILEIO=1 -DRT_EMBED_TEX=1 \
        ${INC_PATH} ${SRC_LIST} ${LIB_PATH} ${LIB_LIST} -o RooT.arm_n900


build_rpiX: RooT_arm_rpi2 RooT_arm_rpi3

strip_rpiX:
	strip RooT.arm_rpi*

clean_rpiX:
	rm RooT.arm_rpi*


RooT_arm_rpi2:
	g++ -O3 -g -pthread -no-pie -march=armv7-a -marm \
        -DRT_LINUX -DRT_ARM -DRT_128=1+2 \
        -DRT_POINTER=32 -DRT_ADDRESS=32 -DRT_ELEMENT=32 -DRT_ENDIAN=0 \
        -DRT_DEBUG=0 -DRT_PATH="../" -DRT_FULLSCREEN=0 \
        -DRT_EMBED_STDOUT=0 -DRT_EMBED_FILEIO=0 -DRT_EMBED_TEX=1 \
        ${INC_PATH} ${SRC_LIST} ${LIB_PATH} ${LIB_LIST} -o RooT.arm_rpi2

RooT_arm_rpi3:
	g++ -O3 -g -pthread -no-pie -march=armv7-a -marm \
        -DRT_LINUX -DRT_ARM -DRT_128=1+2+4 \
        -DRT_POINTER=32 -DRT_ADDRESS=32 -DRT_ELEMENT=32 -DRT_ENDIAN=0 \
        -DRT_DEBUG=0 -DRT_PATH="../" -DRT_FULLSCREEN=0 \
        -DRT_EMBED_STDOUT=0 -DRT_EMBED_FILEIO=0 -DRT_EMBED_TEX=1 \
        ${INC_PATH} ${SRC_LIST} ${LIB_PATH} ${LIB_LIST} -o RooT.arm_rpi3


clang_rpiX: RooT.arm_rpi2 RooT.arm_rpi3

RooT.arm_rpi2:
	clang++ -O3 -g -pthread -march=armv7-a -marm \
        -Wno-unknown-warning-option \
        -Wno-shift-negative-value -Wno-shift-op-parentheses \
        -Wno-logical-op-parentheses -Wno-bitwise-op-parentheses \
        -DRT_LINUX -DRT_ARM -DRT_128=1+2 \
        -DRT_POINTER=32 -DRT_ADDRESS=32 -DRT_ELEMENT=32 -DRT_ENDIAN=0 \
        -DRT_DEBUG=0 -DRT_PATH="../" -DRT_FULLSCREEN=0 \
        -DRT_EMBED_STDOUT=0 -DRT_EMBED_FILEIO=0 -DRT_EMBED_TEX=1 \
        ${INC_PATH} ${SRC_LIST} ${LIB_PATH} ${LIB_LIST} -o RooT.arm_rpi2

RooT.arm_rpi3:
	clang++ -O3 -g -pthread -march=armv7-a -marm \
        -Wno-unknown-warning-option \
        -Wno-shift-negative-value -Wno-shift-op-parentheses \
        -Wno-logical-op-parentheses -Wno-bitwise-op-parentheses \
        -DRT_LINUX -DRT_ARM -DRT_128=1+2+4 \
        -DRT_POINTER=32 -DRT_ADDRESS=32 -DRT_ELEMENT=32 -DRT_ENDIAN=0 \
        -DRT_DEBUG=0 -DRT_PATH="../" -DRT_FULLSCREEN=0 \
        -DRT_EMBED_STDOUT=0 -DRT_EMBED_FILEIO=0 -DRT_EMBED_TEX=1 \
        ${INC_PATH} ${SRC_LIST} ${LIB_PATH} ${LIB_LIST} -o RooT.arm_rpi3


# Prerequisites for the build:
# native-compiler for ARMv7 is installed and in the PATH variable.
# sudo apt-get update
# sudo apt-get install g++ libxext-dev (on ARMv7 host or QEMU system mode)
#
# Building/running RooT demo:
# make -f RooT_make_arm.mk
# ./RooT.arm_v1 (on ARMv7 host or QEMU system mode)
# (has been tested on Raspberry Pi 2/3 target host system with Raspbian/Ubuntu)
# (SIMD and CORE tests pass in QEMU linux-user mode, check test subfolder)

# g++ compilation works on Raspbian/Ubuntu without PIE-mode (-no-pie >= g++-5)
# clang compilation takes much longer prior to 3.8 (older Ubuntu 14.04/Mint 17)
# sudo apt-get update (on Ubuntu add "universe" to "main" /etc/apt/sources.list)
# sudo apt-get install clang libxext-dev (on ARMv7 host or QEMU system mode)
# make -f RooT_make_arm.mk clang

# RooT demo uses runtime SIMD target selection, multiple can be specified above
# on RISC targets top value above is chosen by default, use -n/-k/-s to override

# For interpretation of SIMD build flags check compatibility layer in rtzero.h

# 1) Nokia N900, Maemo 5 scratchbox: -DRT_FULLSCREEN=1 -DRT_EMBED_FILEIO=1
# 2) Raspberry Pi 2, Raspbian: clang++ -DRT_128=1+2
# 3) Raspberry Pi 3, Raspbian: clang++ -DRT_128=1+2+4
