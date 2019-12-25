
INC_PATH =                          \
        -I../core/config/

SRC_LIST =                          \
        simd_test.cpp

simd_test:
	g++ -O3 -g -static -march=armv7-a -marm \
        -DRT_LINUX -DRT_ARM -DRT_DEBUG=1 \
        ${INC_PATH} ${SRC_LIST} -o simd_test.arm
