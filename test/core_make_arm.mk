INC_PATH =                          \
        -I../core/config/           \
        -I../core/engine/           \
        -I../core/system/           \
        -I../core/tracer/           \
        -I../data/materials/        \
        -I../data/objects/          \
        -I../data/textures/         \
        -Iscenes/

SRC_LIST =                          \
        ../core/engine/engine.cpp   \
        ../core/engine/object.cpp   \
        ../core/engine/rtgeom.cpp   \
        ../core/engine/rtimag.cpp   \
        ../core/system/system.cpp   \
        ../core/tracer/tracer.cpp   \
        core_test.cpp

core_test:
	arm-linux-gnueabi-g++ -O3 -g -static -march=armv7-a -marm \
        -DRT_LINUX -DRT_ARM -DRT_DEBUG=1 -DRT_PATH="../" \
        -DRT_EMBED_STDOUT=0 -DRT_EMBED_FILEIO=0 -DRT_EMBED_TEX=1 \
        ${INC_PATH} ${SRC_LIST} -o core_test.arm

# Build target above is suitable for Maemo/N900 and DEB-based cross-compilation.
# Other (than DEB-based) Linux systems may have different name for the compiler.
# For native builds on ARMv7 (Raspberry Pi 2) use plain g++ reference.
# For Maemo/N900 build enable: -DRT_EMBED_FILEIO=1.
