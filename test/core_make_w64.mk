
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
        ../core/tracer/tracer_256v1.cpp \
        ../core/tracer/tracer_256v2.cpp \
        core_test.cpp

LIB_PATH =

LIB_LIST =                              \
        -lm                             \
        -lstdc++

core_test:
	g++ -O3 -g -static -m64 \
        -DRT_WIN64 -DRT_X64 -DRT_128=1+2+4 -DRT_256=1+2 \
        -DRT_POINTER=64 -DRT_ADDRESS=32 -DRT_ENDIAN=0 \
        -DRT_DEBUG=0 -DRT_PATH="../" \
        -DRT_EMBED_STDOUT=0 -DRT_EMBED_FILEIO=0 -DRT_EMBED_TEX=1 \
        ${INC_PATH} ${SRC_LIST} ${LIB_PATH} ${LIB_LIST} -o core_test_w64_32.exe

# Prerequisites for the build:
# TDM64-GCC compiler for Win32/64 is installed and in the PATH variable.
# Download tdm64-gcc-5.1.0-2.exe from sourceforge and run the installer.
#
# Use "MinGW Command Prompt" from "Windows Start Menu" under "TDM-GCC-64".
# Works with regular "cmd" command prompt too after TDM64-GCC installation.
#
# Building/running CORE test:
# run core_make_w64.bat file or
# mingw32-make -f core_make_w64.mk
# core_test_w64_32.exe -i -a
# (should produce antialiased (-a) images (-i) in the ../dump subfolder)

# Experimental 64/32-bit hybrid mode is enabled by default
# until full 64-bit support is implemented in the framework.
