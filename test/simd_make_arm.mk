INC_PATH =                          \
        -I../core/

SRC_LIST =                          \
        simd_test.cpp

simd_test:
	g++ -O3 -g -static -march=armv7-a -marm \
        -DRT_LINUX -DRT_ARM ${INC_PATH} ${SRC_LIST} -o simd_test.arm
