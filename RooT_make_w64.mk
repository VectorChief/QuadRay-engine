
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
        core/tracer/tracer_128v2.cpp    \
        core/tracer/tracer_128v4.cpp    \
        core/tracer/tracer_256v1.cpp    \
        core/tracer/tracer_256v2.cpp    \
        RooT_win32.cpp

LIB_PATH =

LIB_LIST =                              \
        -lm                             \
        -lstdc++                        \
        -lgdi32


build: RooT_w64_32 RooT_w64f32 RooT_w64f64

strip:
	strip RooT_w64*.exe

clean:
	rm RooT_w64*.exe


RooT_w64_32:
	g++ -O3 -g -static -m64 \
        -DRT_WIN64 -DRT_X64 -DRT_128=2+4 -DRT_256=1+2 \
        -DRT_POINTER=64 -DRT_ADDRESS=32 -DRT_ELEMENT=32 -DRT_ENDIAN=0 \
        -DRT_DEBUG=0 -DRT_PATH="./" -DRT_FULLSCREEN=0 \
        -DRT_EMBED_STDOUT=0 -DRT_EMBED_FILEIO=0 -DRT_EMBED_TEX=1 \
        ${INC_PATH} ${SRC_LIST} ${LIB_PATH} ${LIB_LIST} -o RooT_w64_32.exe

RooT_w64f32:
	g++ -O3 -g -static -m64 \
        -DRT_WIN64 -DRT_X64 -DRT_128=2+4 -DRT_256=1+2 \
        -DRT_POINTER=64 -DRT_ADDRESS=64 -DRT_ELEMENT=32 -DRT_ENDIAN=0 \
        -DRT_DEBUG=0 -DRT_PATH="./" -DRT_FULLSCREEN=0 \
        -DRT_EMBED_STDOUT=0 -DRT_EMBED_FILEIO=0 -DRT_EMBED_TEX=1 \
        ${INC_PATH} ${SRC_LIST} ${LIB_PATH} ${LIB_LIST} -o RooT_w64f32.exe

RooT_w64f64:
	g++ -O3 -g -static -m64 \
        -DRT_WIN64 -DRT_X64 -DRT_128=2+4 -DRT_256=1+2 \
        -DRT_POINTER=64 -DRT_ADDRESS=64 -DRT_ELEMENT=64 -DRT_ENDIAN=0 \
        -DRT_DEBUG=0 -DRT_PATH="./" -DRT_FULLSCREEN=0 \
        -DRT_EMBED_STDOUT=0 -DRT_EMBED_FILEIO=0 -DRT_EMBED_TEX=1 \
        ${INC_PATH} ${SRC_LIST} ${LIB_PATH} ${LIB_LIST} -o RooT_w64f64.exe


# Prerequisites for the build:
# TDM64-GCC compiler for Win32/64 is installed and in the PATH variable.
# Download tdm64-gcc-5.1.0-2.exe from sourceforge and run the installer.
#
# Building/running CORE test:
# run RooT_make_w64.bat from Windows UI or
# run the following from Command Prompt "cmd":
# mingw32-make -f RooT_make_w64.mk
# RooT_w64f32.exe

# 64/32-bit (ptr/adr) hybrid mode compatible with native 64-bit ABI
# is available for the original pure 32-bit ISA using 64-bit pointers,
# use (replace): RT_ADDRESS=32, rename the binary to RooT_w64_32.exe

# 64-bit packed SIMD mode (fp64/int64) is supported on 64-bit targets,
# but currently requires addresses to be 64-bit as well (RT_ADDRESS=64),
# use (replace): RT_ELEMENT=64, rename the binary to RooT_w64f64.exe
