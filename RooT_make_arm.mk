
INC_PATH =                              \
        -Icore/config/                  \
        -Icore/engine/                  \
        -Icore/system/                  \
        -Icore/tracer/                  \
        -Idata/materials/               \
        -Idata/objects/                 \
        -Idata/scenes/                  \
        -Idata/textures/

SRC_LIST =                              \
        core/engine/engine.cpp          \
        core/engine/object.cpp          \
        core/engine/rtgeom.cpp          \
        core/engine/rtimag.cpp          \
        core/system/system.cpp          \
        core/tracer/tracer.cpp          \
        core/tracer/tracer_128v1.cpp    \
        core/tracer/tracer_128v2.cpp    \
        core/tracer/tracer_128v4.cpp    \
        RooT_linux.cpp

LIB_PATH =

LIB_LIST =                              \
        -lm                             \
        -lstdc++                        \
        -lX11                           \
        -lXext                          \
        -lpthread


build: RooT_arm

strip:
	arm-linux-gnueabi-strip RooT.arm

clean:
	rm RooT.arm


RooT_arm:
	arm-linux-gnueabi-g++ -O3 -g \
        -DRT_LINUX -DRT_ARM -DRT_128=1 \
        -DRT_POINTER=32 -DRT_ADDRESS=32 -DRT_ELEMENT=32 -DRT_ENDIAN=0 \
        -DRT_DEBUG=0 -DRT_PATH="./" -DRT_FULLSCREEN=0 \
        -DRT_EMBED_STDOUT=0 -DRT_EMBED_FILEIO=0 -DRT_EMBED_TEX=1 \
        ${INC_PATH} ${SRC_LIST} ${LIB_PATH} ${LIB_LIST} -o RooT.arm


build_n900: RooT_arm_n900

strip_n900:
	arm-linux-gnueabi-strip RooT.arm_n900

clean_n900:
	rm RooT.arm_n900


RooT_arm_n900:
	arm-linux-gnueabi-g++ -O3 -g \
        -DRT_LINUX -DRT_ARM -DRT_128=1 \
        -DRT_POINTER=32 -DRT_ADDRESS=32 -DRT_ELEMENT=32 -DRT_ENDIAN=0 \
        -DRT_DEBUG=0 -DRT_PATH="./" -DRT_FULLSCREEN=1 \
        -DRT_EMBED_STDOUT=0 -DRT_EMBED_FILEIO=1 -DRT_EMBED_TEX=1 \
        ${INC_PATH} ${SRC_LIST} ${LIB_PATH} ${LIB_LIST} -o RooT.arm_n900


build_rpiX: RooT_arm_rpi2 RooT_arm_rpi3

strip_rpiX:
	arm-linux-gnueabihf-strip RooT.arm_rpi*

clean_rpiX:
	rm RooT.arm_rpi*


RooT_arm_rpi2:
	arm-linux-gnueabihf-g++ -O3 -g \
        -DRT_LINUX -DRT_ARM -DRT_128=1+2 \
        -DRT_POINTER=32 -DRT_ADDRESS=32 -DRT_ELEMENT=32 -DRT_ENDIAN=0 \
        -DRT_DEBUG=0 -DRT_PATH="./" -DRT_FULLSCREEN=0 \
        -DRT_EMBED_STDOUT=0 -DRT_EMBED_FILEIO=0 -DRT_EMBED_TEX=1 \
        ${INC_PATH} ${SRC_LIST} ${LIB_PATH} ${LIB_LIST} -o RooT.arm_rpi2

RooT_arm_rpi3:
	arm-linux-gnueabihf-g++ -O3 -g \
        -DRT_LINUX -DRT_ARM -DRT_128=1+2+4 \
        -DRT_POINTER=32 -DRT_ADDRESS=32 -DRT_ELEMENT=32 -DRT_ENDIAN=0 \
        -DRT_DEBUG=0 -DRT_PATH="./" -DRT_FULLSCREEN=0 \
        -DRT_EMBED_STDOUT=0 -DRT_EMBED_FILEIO=0 -DRT_EMBED_TEX=1 \
        ${INC_PATH} ${SRC_LIST} ${LIB_PATH} ${LIB_LIST} -o RooT.arm_rpi3


# Prerequisites for the build:
# native-compiler for ARMv7 is installed and in the PATH variable.
# sudo apt-get install g++ libxext-dev (on ARMv7 host or QEMU system mode)
#
# Building/running RooT demo:
# make -f RooT_make_arm.mk
# ./RooT.arm (on ARMv7 host or QEMU system mode)

# 0) Build flags above are intended for default "vanilla" ARMv7 target, while
# settings suitable for specific hardware platforms are given below (replace).
# 1) Nokia N900, Maemo 5 scratchbox: -DRT_FULLSCREEN=1 -DRT_EMBED_FILEIO=1
# 2) Raspberry Pi 2, Raspbian: arm-linux-gnueabihf-g++ -DRT_128=1+2
# 3) Raspberry Pi 3, Raspbian: arm-linux-gnueabihf-g++ -DRT_128=1+2+4
