gcc coll.c -o coll_gen.exe -std=c99 -O3 -Wall -Wextra -Werror -fmax-errors=1 -Wno-unused-function -march=core2 -D COMPILING_FOR_X86_64_WITH_SSSE3 -D VERSION_FOR_AVX_512F_IS_AVAILABLE
gcc coll.c -o coll_avx2.exe -std=c99 -O3 -Wall -Wextra -Werror -fmax-errors=1 -Wno-unused-function -march=haswell -D COMPILING_FOR_X86_64_WITH_AVX2 -D VERSION_FOR_AVX_512F_IS_AVAILABLE
