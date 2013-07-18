INC_PATH =                          \
        -I../core/config            \
        -I../core/tracer

SRC_LIST =                          \
        simd_test.cpp

simd_test:
	g++ -O3 -g -DRT_X86             \
        ${INC_PATH} ${SRC_LIST} -o simd_test.x86
