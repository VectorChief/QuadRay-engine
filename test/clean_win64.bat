:: Intended for x86_64 Windows build environment
:: with TDM64-GCC compiler installed (64-bit Windows 7 SP1 tested)

mingw32-make -f simd_make_w64.mk clean


mingw32-make -f core_make_w64.mk clean


cd ../root

mingw32-make -f RooT_make_w64.mk clean

cd ../test
