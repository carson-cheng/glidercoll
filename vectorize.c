// Suggested compiles:
// -march=core2 -D NO_AVX2 -D COMPILING_FOR_X86_64_WITH_SSSE3
// -march=haswell -D COMPILING_FOR_X86_64_WITH_AVX2
// This should generate code that run on any processor with AVX-512F support. It has been tested that this indeed generates AVX-512 instructions
// -march=broadwell -mavx512f -D COMPILING_FOR_X86_64_WITH_AVX_512F
// Add -D VERSION_FOR_AVX_512F_IS_AVAILABLE if applicable
// If compiling for another target than these three, use -D TARGET_ARCH_VECTOR_BYTE_SIZE=<size> instead

#ifndef TARGET_ARCH_VECTOR_BYTE_SIZE
	#ifdef COMPILING_FOR_X86_64_WITH_SSSE3
		#define TARGET_ARCH_VECTOR_BYTE_SIZE 16
	#endif
	#ifdef COMPILING_FOR_X86_64_WITH_AVX2
		#define TARGET_ARCH_VECTOR_BYTE_SIZE 32
	#endif
	#ifdef COMPILING_FOR_X86_64_WITH_AVX_512F
		#define TARGET_ARCH_VECTOR_BYTE_SIZE 64
	#endif
#endif

#ifndef TARGET_ARCH_VECTOR_BYTE_SIZE
	#error "TARGET_ARCH_VECTOR_BYTE_SIZE is not defined"
#endif

#ifndef VERSION_FOR_AVX_512F_IS_AVAILABLE
	#define VERSION_FOR_AVX_512F_IS_AVAILABLE FALSE
#endif

#ifdef __GNUC__
	#define dont_replace_loop_with_library_call __attribute__((optimize("-fno-tree-loop-distribute-patterns")))
#endif
#ifdef __INTEL_COMPILER
	#define dont_replace_loop_with_library_call
#endif
#ifndef dont_replace_loop_with_library_call
	#define dont_replace_loop_with_library_call
#endif

static never_inline int verify_cpu_type (void)
{
	static const char *feature_name [4] = {"Generic", "SSSE3", "AVX2", "AVX-512F"};

	int required_cpu_feature_class = 0;
	
	#ifdef COMPILING_FOR_X86_64_WITH_AVX_512F
		required_cpu_feature_class = 3;
	#endif
	#ifdef COMPILING_FOR_X86_64_WITH_AVX2
		required_cpu_feature_class = 2;
	#endif
	#ifdef COMPILING_FOR_X86_64_WITH_SSSE3
		required_cpu_feature_class = 1;
	#endif
	
	int available_cpu_feature_class = -1;
	
	// Note that an undefined preprocessor symbol evaluates to 0 in a preprocessor expression
	#if __GNUC__ > 4 || (__GNUC__ == 4 && __GNUC_MINOR__ >= 8)
		if (__builtin_cpu_supports ("avx512f"))
			available_cpu_feature_class = 3;
		else if (__builtin_cpu_supports ("avx2"))
			available_cpu_feature_class = 2;
		else if (__builtin_cpu_supports ("ssse3"))
			available_cpu_feature_class = 1;
		else
			available_cpu_feature_class = 0;
	#endif
	
	int best_available_executable_cpu_feature_class = 2;
	#if VERSION_FOR_AVX_512F_IS_AVAILABLE
		best_available_executable_cpu_feature_class = 3;
	#endif
	
	int allowed_to_run = TRUE;
	
	if (available_cpu_feature_class == -1 && required_cpu_feature_class > 1)
	{
		fprintf (stderr, "Warning: This executable was compiled for CPUs with support for %s,\n", feature_name [required_cpu_feature_class]);
		fprintf (stderr, "         but it is not checked if this CPU supports that. If the program\n");
		fprintf (stderr, "         crashes, please try the executable that require at most %s instead\n", feature_name [required_cpu_feature_class - 1]);
	}
	else if (required_cpu_feature_class > 0 && available_cpu_feature_class == 0)
	{
		fprintf (stderr, "Error: This CPU doesn't support %s, but all executable versions of this\n", feature_name [1]);
		fprintf (stderr, "       program require that.\n");
		allowed_to_run = FALSE;
	}
	else if (available_cpu_feature_class != -1 && required_cpu_feature_class > available_cpu_feature_class)
	{
		fprintf (stderr, "Error: This executable was compiled for CPUs with support for %s,\n", feature_name [required_cpu_feature_class]);
		fprintf (stderr, "       but this CPU doesn't support that. Please use an executable\n");
		fprintf (stderr, "       that require at most %s instead.\n", feature_name [available_cpu_feature_class]);
		allowed_to_run = FALSE;
	}
	else if (required_cpu_feature_class < available_cpu_feature_class && required_cpu_feature_class < best_available_executable_cpu_feature_class)
	{
		fprintf (stderr, "Note: This executable was compiled for CPUs without support for %s,\n", feature_name [available_cpu_feature_class]);
		fprintf (stderr, "      but this CPU does support %s. The program will run faster\n", feature_name [available_cpu_feature_class]);
		fprintf (stderr, "      if the %s executable is used instead.\n", feature_name [available_cpu_feature_class]);
	}
	
	return allowed_to_run;
}

void try_v (void)
{
	int result = verify_cpu_type ();
	fprintf (stderr, "result = %s\n\n", result ? "OK" : "FAIL");
}
