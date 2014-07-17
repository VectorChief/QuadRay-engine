
INC_PATH =                          \
        -Icore/config/              \
        -Icore/engine/              \
        -Icore/system/              \
        -Icore/tracer/              \
        -Idata/materials/           \
        -Idata/objects/             \
        -Idata/scenes/              \
        -Idata/textures/

LIB_PATH =


SRC_LIST =                          \
        core/engine/engine.cpp      \
        core/engine/object.cpp      \
        core/engine/rtgeom.cpp      \
        core/engine/rtimag.cpp      \
        core/system/system.cpp      \
        core/tracer/tracer.cpp      \
        RooT_linux.cpp

LIB_LIST =                          \
        -lX11                       \
        -lXext

RooT:
	g++ -O3 -g -m32 \
        -DRT_X86 -DRT_DEBUG=0 -DRT_EMBED=0 -DRT_FULLSCREEN=0 \
        ${INC_PATH} ${SRC_LIST} ${LIB_PATH} ${LIB_LIST} -o RooT.x86
