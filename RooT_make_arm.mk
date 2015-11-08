
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
        -lXext                      \
        -lpthread

RooT:
	g++ -O3 -g -fexceptions \
        -DRT_PATH="./" \
        -DRT_LINUX -DRT_ARM -DRT_128 -DRT_DEBUG=0 -DRT_FULLSCREEN=1 \
        -DRT_EMBED_STDOUT=0 -DRT_EMBED_FILEIO=1 -DRT_EMBED_TEX=1 \
        ${INC_PATH} ${SRC_LIST} ${LIB_PATH} ${LIB_LIST} -o RooT.arm
