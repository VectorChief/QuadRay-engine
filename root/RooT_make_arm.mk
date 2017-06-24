
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


build: RooT_arm

strip:
	arm-linux-gnueabi-strip RooT.arm

clean:
	rm RooT.arm


RooT_arm:
	clang++ -O3 -g -pthread -march=armv7-a -marm \
        -Wno-shift-negative-value -Wno-shift-op-parentheses \
        -Wno-logical-op-parentheses -Wno-bitwise-op-parentheses \
        -DRT_LINUX -DRT_ARM -DRT_128=1 -Wno-unknown-warning-option \
        -DRT_POINTER=32 -DRT_ADDRESS=32 -DRT_ELEMENT=32 -DRT_ENDIAN=0 \
        -DRT_DEBUG=0 -DRT_PATH="../" -DRT_FULLSCREEN=0 \
        -DRT_EMBED_STDOUT=0 -DRT_EMBED_FILEIO=0 -DRT_EMBED_TEX=1 \
        ${INC_PATH} ${SRC_LIST} ${LIB_PATH} ${LIB_LIST} -o RooT.arm


build_n900: RooT_arm_n900

strip_n900:
	arm-linux-gnueabi-strip RooT.arm_n900

clean_n900:
	rm RooT.arm_n900


RooT_arm_n900:
	arm-linux-gnueabi-g++ -O3 -g -pthread -march=armv7-a -marm \
        -DRT_LINUX -DRT_ARM -DRT_128=1 \
        -DRT_POINTER=32 -DRT_ADDRESS=32 -DRT_ELEMENT=32 -DRT_ENDIAN=0 \
        -DRT_DEBUG=0 -DRT_PATH="../" -DRT_FULLSCREEN=1 \
        -DRT_EMBED_STDOUT=0 -DRT_EMBED_FILEIO=1 -DRT_EMBED_TEX=1 \
        ${INC_PATH} ${SRC_LIST} ${LIB_PATH} ${LIB_LIST} -o RooT.arm_n900


build_rpiX: RooT_arm_rpi2 RooT_arm_rpi3

strip_rpiX:
	arm-linux-gnueabihf-strip RooT.arm_rpi*

clean_rpiX:
	rm RooT.arm_rpi*


RooT_arm_rpi2:
	clang++ -O3 -g -pthread -march=armv7-a -marm \
        -Wno-shift-negative-value -Wno-shift-op-parentheses \
        -Wno-logical-op-parentheses -Wno-bitwise-op-parentheses \
        -DRT_LINUX -DRT_ARM -DRT_128=1+2 -Wno-unknown-warning-option \
        -DRT_POINTER=32 -DRT_ADDRESS=32 -DRT_ELEMENT=32 -DRT_ENDIAN=0 \
        -DRT_DEBUG=0 -DRT_PATH="../" -DRT_FULLSCREEN=0 \
        -DRT_EMBED_STDOUT=0 -DRT_EMBED_FILEIO=0 -DRT_EMBED_TEX=1 \
        ${INC_PATH} ${SRC_LIST} ${LIB_PATH} ${LIB_LIST} -o RooT.arm_rpi2

RooT_arm_rpi3:
	clang++ -O3 -g -pthread -march=armv7-a -marm \
        -Wno-shift-negative-value -Wno-shift-op-parentheses \
        -Wno-logical-op-parentheses -Wno-bitwise-op-parentheses \
        -DRT_LINUX -DRT_ARM -DRT_128=1+2+4 -Wno-unknown-warning-option \
        -DRT_POINTER=32 -DRT_ADDRESS=32 -DRT_ELEMENT=32 -DRT_ENDIAN=0 \
        -DRT_DEBUG=0 -DRT_PATH="../" -DRT_FULLSCREEN=0 \
        -DRT_EMBED_STDOUT=0 -DRT_EMBED_FILEIO=0 -DRT_EMBED_TEX=1 \
        ${INC_PATH} ${SRC_LIST} ${LIB_PATH} ${LIB_LIST} -o RooT.arm_rpi3


# Prerequisites for the build:
# native-compiler for ARMv7 is installed and in the PATH variable.
# sudo apt-get install clang libxext-dev (on ARMv7 host or QEMU system mode)
# (clang compilation takes much longer prior to 3.8: older Ubuntu 14.04/Mint 17)
# (g++ on Ubuntu 17.10++/Debian 9 has PIE-by-default/link-errors => use clang++)
# http://www.phoronix.com/scan.php?page=news_item&px=Ubuntu-17.10-PIE-SecureBoot
# http://wiki.debian.org/Hardening/PIEByDefaultTransition
#
# Building/running RooT demo:
# make -f RooT_make_arm.mk
# ./RooT.arm (on ARMv7 host or QEMU system mode)
# (has been tested on Raspberry Pi 2/3 target host system with Raspbian/Ubuntu)
# (SIMD and CORE tests pass in QEMU linux-user mode, check test subfolder)

# g++ compilation works on Raspbian/Ubuntu without PIE-mode, use (replace):
# g++ (in place of clang++)
# sudo apt-get install g++ libxext-dev (on ARMv7 host)

# RooT demo uses runtime SIMD target selection, multiple can be specified above
# on RISC targets top value above is chosen by default, use -n/-k/-s to override

# For interpretation of SIMD build flags check compatibility layer in rtzero.h

# 1) Nokia N900, Maemo 5 scratchbox: -DRT_FULLSCREEN=1 -DRT_EMBED_FILEIO=1
# 2) Raspberry Pi 2, Raspbian: clang++ -DRT_128=1+2
# 3) Raspberry Pi 3, Raspbian: clang++ -DRT_128=1+2+4
