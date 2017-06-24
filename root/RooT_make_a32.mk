
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
        ../core/tracer/tracer_256v1.cpp     \
        RooT_linux.cpp

LIB_PATH =

LIB_LIST =                                  \
        -lm                                 \
        -lstdc++                            \
        -lX11                               \
        -lXext                              \
        -lpthread


build: RooT_a32

strip:
	aarch64-linux-gnu-strip RooT.a32

clean:
	rm RooT.a32


RooT_a32:
	aarch64-linux-gnu-g++ -O3 -g -pthread -mabi=ilp32 \
        -DRT_LINUX -DRT_A32 -DRT_128=1 -DRT_256=1 \
        -DRT_POINTER=32 -DRT_ADDRESS=32 -DRT_ELEMENT=32 -DRT_ENDIAN=0 \
        -DRT_DEBUG=0 -DRT_PATH="../" -DRT_FULLSCREEN=0 \
        -DRT_EMBED_STDOUT=0 -DRT_EMBED_FILEIO=0 -DRT_EMBED_TEX=1 \
        ${INC_PATH} ${SRC_LIST} ${LIB_PATH} ${LIB_LIST} -o RooT.a32

RooT.a32:
	clang++ -O3 -g -pthread -mabi=ilp32 -Wno-unknown-warning-option \
        -Wno-shift-negative-value -Wno-shift-op-parentheses \
        -Wno-logical-op-parentheses -Wno-bitwise-op-parentheses \
        -DRT_LINUX -DRT_A32 -DRT_128=1 -DRT_256=1 \
        -DRT_POINTER=32 -DRT_ADDRESS=32 -DRT_ELEMENT=32 -DRT_ENDIAN=0 \
        -DRT_DEBUG=0 -DRT_PATH="../" -DRT_FULLSCREEN=0 \
        -DRT_EMBED_STDOUT=0 -DRT_EMBED_FILEIO=0 -DRT_EMBED_TEX=1 \
        ${INC_PATH} ${SRC_LIST} ${LIB_PATH} ${LIB_LIST} -o RooT.a32


# Prerequisites for the build:
# multilib-compiler for AArch64 is installed and in the PATH variable,
# plus X11/Xext libs for ILP32 ABI on AArch64 host or QEMU system mode.
# (recent upstream g++-5-aarch64 series may not fully support ILP32 ABI)
# (g++ on Ubuntu 17.10++/Debian 9 has PIE-by-default/link-errors => use clang++)
# http://www.phoronix.com/scan.php?page=news_item&px=Ubuntu-17.10-PIE-SecureBoot
# http://wiki.debian.org/Hardening/PIEByDefaultTransition
#
# Building/running RooT demo:
# make -f RooT_make_a32.mk
# ./RooT.a32 (on AArch64 host or QEMU system mode with ILP32 X11/Xext libs)
# (hasn't been verified yet due to lack of available libs)

# Clang compilation works too (takes much longer prior to 3.8), use (replace):
# clang++ -Wno-logical-op-parentheses -Wno-bitwise-op-parentheses
# sudo apt-get install clang (requires g++-multilib for non-native ABI)

# RooT demo uses runtime SIMD target selection, multiple can be specified above
# on RISC targets top value above is chosen by default, use -n/-k/-s to override
# 256-bit SIMD is achieved by combining pairs of 128-bit registers/instructions

# For interpretation of SIMD build flags check compatibility layer in rtzero.h

# 32-bit ABI hasn't been fully tested yet due to lack of available libs,
# check out 64/32-bit (ptr/adr) hybrid mode for 64-bit ABI in RooT_make_a64.mk
