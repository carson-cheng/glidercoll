// This is meant to be a minimal set of standard include files for a simple program
#include <inttypes.h>
#include <string.h>
#include <stdlib.h>
#include <stdio.h>

#define FALSE 0
#define TRUE 1

#define i8 int8_t
#define u8 uint8_t
#define i16 int16_t
#define u16 uint16_t
#define i32 int32_t
#define u32 uint32_t
#define i64 int64_t
#define u64 uint64_t

#define i8_MIN INT8_MIN
#define ABS_i8_MIN 128u
#define i8_MAX INT8_MAX
#define u8_MAX UINT8_MAX
#define i16_MIN INT16_MIN
#define ABS_i16_MIN 32768u
#define i16_MAX INT16_MAX
#define u16_MAX UINT16_MAX
#define i32_MIN INT32_MIN
#define ABS_i32_MIN 2147483648u
#define i32_MAX INT32_MAX
#define u32_MAX UINT32_MAX
#define i64_MIN INT64_MIN
#define ABS_i64_MIN 9223372036854775808u
#define i64_MAX INT64_MAX
#define u64_MAX UINT64_MAX

// Integer types that are at least 32 bits wide, but not narrower than an int, to make sure no integer promotions are performed
// The intention is to cover the case where ints are 64 bits wide so that an int32_t would be promoted to 64 bits in arithmetic operations
// int_least32_t is not guaranteed not to be narrower than an int. "up" is short for unpromotable
#if LONG_MAX == 32767
	#define upu32 unsigned long long
	#define upi32 long long
#elif INT_MAX == 32767
	#define upu32 unsigned long
	#define upi32 long
#else
	#define upu32 unsigned int
	#define upi32 int
#endif

#define TYPICAL_CACHE_LINE_BYTE_SIZE 64
#define MAX_LIKELY_CACHE_LINE_BYTE_SIZE 256
#define TYPICAL_MEMORY_PAGE_BYTE_SIZE 4096

#ifdef __GNUC__
	#define never_inline __attribute__((noinline))
	#define always_inline inline __attribute__((always_inline))
#endif
#ifdef __INTEL_COMPILER
	#define never_inline __declspec(noinline)
	#define always_inline __forceinline
#endif
#ifndef never_inline
	#define never_inline
#endif
#ifndef always_inline
	#define always_inline inline
#endif


// ffsc = Function failed sanity check
static never_inline int ffsc (const char *fname)
{
	fprintf (stderr, "%s failed in function entry sanity check\n", (fname ? fname : "<unknown function namn>"));
	return FALSE;
}

static never_inline void *ffsc_p (const char *fname)
{
	fprintf (stderr, "%s failed in function entry sanity check\n", (fname ? fname : "<unknown function namn>"));
	return NULL;
}

static never_inline i32 ffsc_i32 (const char *fname, i32 fail_value)
{
	fprintf (stderr, "%s failed in function entry sanity check\n", (fname ? fname : "<unknown function namn>"));
	return fail_value;
}

// The point with this is to allow better optimization when divisor is known at compile time
static always_inline i32 div_pos_i32 (i32 dividend, i32 divisor)
{
	return (i32) ((u32) dividend / (u32) divisor);
}

static always_inline u32 abs_i32_to_u32 (i32 op)
{
	if (op >= 0)
		return (u32) op;
	else if (op == i32_MIN)
		return (u32) ABS_i32_MIN;
	else
		return (u32) -op;
}

static always_inline u64 abs_i64_to_u64 (i64 op)
{
	if (op >= 0)
		return (u64) op;
	else if (op == i64_MIN)
		return (u64) ABS_i64_MIN;
	else
		return (u64) -op;
}

static always_inline i32 sign_i32 (i32 op)
{
	if (op > 0)
		return 1;
	else if (op < 0)
		return -1;
	else
		return 0;
}

static always_inline i64 sign_i64 (i64 op)
{
	if (op > 0)
		return 1;
	else if (op < 0)
		return -1;
	else
		return 0;
}

static always_inline u32 lower_of_u32 (u32 op1, u32 op2)
{
	if (op1 <= op2)
		return op1;
	else
		return op2;
}

static always_inline u32 higher_of_u32 (u32 op1, u32 op2)
{
	if (op1 >= op2)
		return op1;
	else
		return op2;
}

static always_inline i32 lower_of_i32 (i32 op1, i32 op2)
{
	if (op1 <= op2)
		return op1;
	else
		return op2;
}

static always_inline i32 higher_of_i32 (i32 op1, i32 op2)
{
	if (op1 >= op2)
		return op1;
	else
		return op2;
}

static always_inline u64 lower_of_u64 (u64 op1, u64 op2)
{
	if (op1 <= op2)
		return op1;
	else
		return op2;
}

static always_inline u64 higher_of_u64 (u64 op1, u64 op2)
{
	if (op1 >= op2)
		return op1;
	else
		return op2;
}

static always_inline i64 lower_of_i64 (i64 op1, i64 op2)
{
	if (op1 <= op2)
		return op1;
	else
		return op2;
}

static always_inline i64 higher_of_i64 (i64 op1, i64 op2)
{
	if (op1 >= op2)
		return op1;
	else
		return op2;
}

// Tests if 0 <= val < range_off. range_off must be non-negative
static always_inline int is_in_zero_based_range_i32 (i32 val, i32 range_off)
{
	return (u32) val < (u32) range_off;
}

static always_inline void init_range_to_empty_i32 (i32 *range_on, i32 *range_off, i32 val)
{
	*range_on = val;
	*range_off = val;
}

static always_inline void init_range_to_include_i32 (i32 *range_on, i32 *range_off, i32 val)
{
	*range_on = val;
	*range_off = val + 1;
}

static always_inline int range_is_empty_i32 (i32 range_on, i32 range_off)
{
	return (range_off <= range_on);
}

static always_inline int is_in_range_i32 (i32 val, i32 range_on, i32 range_off)
{
	return (val >= range_on && val < range_off);
}

static always_inline void expand_range_to_include_i32 (i32 *range_on, i32 *range_off, i32 val)
{
	if (val < *range_on)
		*range_on = val;
	else if (val + 1 > *range_off)
		*range_off = val + 1;
}

static always_inline u32 align_down_u32 (u32 arg, u32 alignment)
{
	return arg & ~(alignment - 1);
}

static always_inline u32 align_up_u32 (u32 arg, u32 alignment)
{
	return (arg + (alignment - 1)) & ~(alignment - 1);
}

// We perform the bitwise operations on signed numbers. This is to avoid converting an unsigned number back to a signed number when returning the result, which is not well defined by the
// C standard if the result would be reinterpreted as negative
// Bitwise operations are well defined for signed integers, but if arg is negative, the result will be correct only if the architecture is using two's complement representation of integers
static always_inline i32 align_down_i32_twos_compl (i32 arg, i32 alignment)
{
// FIXME
//	fprintf (stderr, "Test that this still works with vectorization after changing the type of alignment from u32 to i32\n");
	return arg & ~(alignment - 1);
}

static always_inline i32 align_up_i32_twos_compl (i32 arg, i32 alignment)
{
//	fprintf (stderr, "Test that this still works after changing alignment from an u32 to an i32\n");
	return (arg + (alignment - 1)) & ~(alignment - 1);
}

static always_inline u64 align_down_u64 (u64 arg, u64 alignment)
{
	return arg & ~(alignment - 1);
}

static always_inline u64 align_up_u64 (u64 arg, u64 alignment)
{
	return (arg + (alignment - 1)) & ~(alignment - 1);
}

static always_inline i64 align_down_i64_twos_compl (i64 arg, i64 alignment)
{
	return arg & ~(alignment - 1);
}

static always_inline i64 align_up_i64_twos_compl (i64 arg, i64 alignment)
{
	return (arg + (alignment - 1)) & ~(alignment - 1);
}

static always_inline int get_bit_u64 (u64 word, int bit_ix)
{
	return (int) ((word >> bit_ix) & (u64) 1);
}

static always_inline void clear_bit_u64 (u64 *word, int bit_ix)
{
	*word = (*word) & ~((u64) 1 << bit_ix);
}

static always_inline void set_bit_u64 (u64 *word, int bit_ix)
{
	*word = (*word) | ((u64) 1 << bit_ix);
}

static always_inline void define_bit_u64 (u64 *word, int bit_ix, int bit_value)
{
	*word = ((*word) & ~((u64) 1 << bit_ix)) | ((u64) bit_value << bit_ix);
}

// About 4.5 clock cycles on Haswell
static always_inline int bit_count_u64 (u64 word)
{
	word = (word & 0x5555555555555555u) + ((word >> 1) & 0x5555555555555555u);
	word = (word & 0x3333333333333333u) + ((word >> 2) & 0x3333333333333333u);
	word = (word & 0x0f0f0f0f0f0f0f0fu) + ((word >> 4) & 0x0f0f0f0f0f0f0f0fu);
	return (word * 0x0101010101010101u) >> 56;
}

#define SIGNIFICANT_BIT_MAGIC_NUMBER 0x19afe5d5b8f9ed27u

static always_inline int most_significant_bit_in_one_below_a_power_of_two_fail_to_m1 (u64 word)
{
	static const i8 magic_table [116] = 
		{-1, -2, 36, -2, -2, -2, 55, -2, -2, -2, 18, -2,  0, -2,  4, 58, -2, 44, 37, -2, -2, -2, -2, -2, 53, -2, 56, -2, -2, -2, 25, -2,
		 27, 19, -2,  8, -2, -2,  1, 50, -2,  5, 29, 59, 32, 21, -2, -2, 45, 38, 10, 62, -2, -2, -2, -2, -2, -2, 35, -2, 54, -2, 17, -2,
		  3, 57, 43, -2, -2, 52, -2, -2, 24, -2, 26,  7, -2, 49, 28, 31, 20, -2,  9, 61, -2, -2, -2, 34, 16,  2, 42, -2, 51, -2, 23,  6,
		 48, 30, -2, 60, -2, 33, 15, 41, 22, 47, -2, -2, 14, 40, 46, 13, 39, 12, 11, 63};
	
	return (int) magic_table [(word * SIGNIFICANT_BIT_MAGIC_NUMBER) >> 57];
}

static always_inline int most_significant_bit_in_one_below_a_power_of_two_fail_to_64 (u64 word)
{
	static const i8 magic_table [116] = 
		{64, -2, 36, -2, -2, -2, 55, -2, -2, -2, 18, -2,  0, -2,  4, 58, -2, 44, 37, -2, -2, -2, -2, -2, 53, -2, 56, -2, -2, -2, 25, -2,
		 27, 19, -2,  8, -2, -2,  1, 50, -2,  5, 29, 59, 32, 21, -2, -2, 45, 38, 10, 62, -2, -2, -2, -2, -2, -2, 35, -2, 54, -2, 17, -2,
		  3, 57, 43, -2, -2, 52, -2, -2, 24, -2, 26,  7, -2, 49, 28, 31, 20, -2,  9, 61, -2, -2, -2, 34, 16,  2, 42, -2, 51, -2, 23,  6,
		 48, 30, -2, 60, -2, 33, 15, 41, 22, 47, -2, -2, 14, 40, 46, 13, 39, 12, 11, 63};
	
	return (int) magic_table [(word * SIGNIFICANT_BIT_MAGIC_NUMBER) >> 57];
}

// About 1.5 clock cycles on Haswell with table in L1 cache
static always_inline int least_significant_bit_u64_fail_to_m1 (u64 word)
{
	if (word != 0)
		word = (word ^ (word - 1));
	
	return most_significant_bit_in_one_below_a_power_of_two_fail_to_m1 (word);
}

static always_inline int least_significant_bit_u64_fail_to_64 (u64 word)
{
	if (word != 0)
		word = (word ^ (word - 1));
	
	return most_significant_bit_in_one_below_a_power_of_two_fail_to_64 (word);
}

// About 5 clock cycles on Haswell with table in L1 cache
static always_inline int most_significant_bit_u64_fail_to_m1 (u64 word)
{
	word |= (word >> 1);
	word |= (word >> 2);
	word |= (word >> 4);
	word |= (word >> 8);
	word |= (word >> 16);
	word |= (word >> 32);
	
	return most_significant_bit_in_one_below_a_power_of_two_fail_to_m1 (word);
}

static always_inline int most_significant_bit_u64_fail_to_64 (u64 word)
{
	word |= (word >> 1);
	word |= (word >> 2);
	word |= (word >> 4);
	word |= (word >> 8);
	word |= (word >> 16);
	word |= (word >> 32);
	
	return most_significant_bit_in_one_below_a_power_of_two_fail_to_64 (word);
}

// GCC recognizes this as a byte reversal followed by other code (that serves to reverse the bits in each byte)
// Earlier versions of GCC had trouble vectorizing all of this when used in a loop, but it works correctly with GCC 8.1.0
static always_inline u64 bit_reverse_u64 (u64 word)
{
	word = (word << 32) | (word >> 32);
	word = ((word & 0xffff0000ffff0000u) >> 16) | ((word & 0x0000ffff0000ffffu) << 16);
	word = ((word & 0xff00ff00ff00ff00u) >>  8) | ((word & 0x00ff00ff00ff00ffu) <<  8);
	word = ((word & 0xf0f0f0f0f0f0f0f0u) >>  4) | ((word & 0x0f0f0f0f0f0f0f0fu) <<  4);
	word = ((word & 0xccccccccccccccccu) >>  2) | ((word & 0x3333333333333333u) <<  2);
	word = ((word & 0xaaaaaaaaaaaaaaaau) >>  1) | ((word & 0x5555555555555555u) <<  1);
	return word;
}

static always_inline i32 round_double_to_i32 (double arg)
{
	if (arg < 0.0)
		return (i32) (arg - 0.5);
	else
		return (i32) (arg + 0.5);
}

static always_inline i64 round_double_to_i64 (double arg)
{
	if (arg < 0.0)
		return (i64) (arg - 0.5);
	else
		return (i64) (arg + 0.5);
}

static always_inline u64 rotate_left_u64 (u64 arg, int rot)
{
	return (arg << rot) | (arg >> (64 - rot));
}

static always_inline u64 rotate_right_u64 (u64 arg, int rot)
{
	return (arg >> rot) | (arg << (64 - rot));
}

static always_inline const void *align_down_const_pointer (const void *p, u64 alignment)
{
	return (const void *) (((uintptr_t) p) & (uintptr_t) ~(alignment - 1));
}

static always_inline const void *align_up_const_pointer (const void *p, u64 alignment)
{
	return (const void *) (((uintptr_t) p + (uintptr_t) (alignment - 1)) & (uintptr_t) ~(alignment - 1));
}

static always_inline void *align_down_pointer (void *p, u64 alignment)
{
	return (void *) (((uintptr_t) p) & (uintptr_t) ~(alignment - 1));
}

static always_inline void *align_up_pointer (void *p, u64 alignment)
{
	return (void *) (((uintptr_t) p + (uintptr_t) (alignment - 1)) & (uintptr_t) ~(alignment - 1));
}

static always_inline void swap_pointers (void **p1, void **p2)
{
	void *temp_p = *p1;
	*p1 = *p2;
	*p2 = temp_p;
}

static never_inline void *mem_alloc (i64 size)
{
	if (size <= 0)
		return ffsc_p (__func__);
	
	void *mem = malloc ((size_t) size);
	if (!mem)
		fprintf (stderr, "%s failed to allocate %" PRId64 "byte\n", __func__, size);
	
	return mem;
}

static never_inline void mem_free (void **mem)
{
	if (!mem)
		return (void) ffsc (__func__);
	
	if (*mem == NULL)
	{
		fprintf (stderr, "Attempt to free from NULL pointer in %s\n", __func__);
		return;
	}
	
	free (*mem);
	*mem = NULL;
}

static always_inline void mem_clear (void *mem, i64 size)
{
	memset (mem, 0, (size_t) size);
}

static always_inline void mem_copy (const void *restrict src, void *restrict dst, i64 size)
{
	memcpy (dst, src, (size_t) size);
}

static always_inline int str_eq (const char *str_1, const char *str_2)
{
	return strcmp (str_1, str_2) == 0;
}

static never_inline i64 str_len (const char *s)
{
	if (!s)
		return ffsc (__func__);
	
	return (i64) strlen (s);
}

static never_inline i64 str_len_limited (const char *s, i64 len_limit)
{
	if (!s || len_limit < 0)
		return ffsc (__func__);
	
	const char *stop_byte = memchr (s, '\0', (size_t) len_limit);
	if (!stop_byte)
		return (i64) -1;
	
	return (i64) (stop_byte - s);
}

static never_inline i64 str_copy (const char *restrict src, char *restrict dst, i64 dst_buf_size)
{
	if (!src || !dst || dst_buf_size <= 0)
	{
		if (dst && dst_buf_size > 0)
			*dst = '\0';
		
		ffsc (__func__);
		return -1;
	}
	
	i64 mem_size = 1 + strlen (src);
	if (mem_size > dst_buf_size)
	{
		*dst = '\0';
		fprintf (stderr, "Buffer overflow in %s\n", __func__);
		return -1;
	}
	
	mem_copy (src, dst, mem_size);
	return mem_size - 1;
}

static never_inline i64 str_copy_mid (const char *restrict src, i64 start_offset, i64 copy_size, char *restrict dst, i64 dst_buf_size)
{
	if (!src || copy_size < 0 || !dst || dst_buf_size <= 0)
	{
		if (dst && dst_buf_size > 0)
			*dst = '\0';
		
		ffsc (__func__);
		return -1;
	}
	
	i64 src_len = -1;
	if (start_offset < 0)
	{
		src_len = strlen (src);
		start_offset += src_len;
	}
	
	if (start_offset < 0)
	{
		*dst = '\0';
		fprintf (stderr, "Requested start offset before string start in %s\n", __func__);
		return -1;
	}
	
	if (src_len == -1)
		src_len = str_len_limited (src, start_offset + lower_of_i64 (copy_size + 1, dst_buf_size + 1));
	
	if (src_len >= 0)
		copy_size = lower_of_i64 (copy_size, src_len - start_offset);
	
	if (copy_size >= dst_buf_size)
	{
		*dst = '\0';
		fprintf (stderr, "Buffer overflow in %s\n", __func__);
		return -1;
	}
	
	if (src_len >= 0 && start_offset > src_len)
	{
		*dst = '\0';
		fprintf (stderr, "Requested start offset after string end in %s\n", __func__);
		return -1;
	}
	
	mem_copy (src + start_offset, dst, copy_size);
	dst [copy_size] = '\0';
	
	return copy_size;
}

static never_inline int allocate_aligned (i64 size, i64 alignment, i64 alignment_offset, int clear, void **allocated_buffer, void **aligned_buffer)
{
	if (aligned_buffer)
		*aligned_buffer = NULL;
	if (allocated_buffer)
		*allocated_buffer = NULL;
	
	if (size == 0 || alignment < 0 || bit_count_u64 ((u64) alignment) != 1 || alignment_offset < 0 || alignment_offset >= alignment || !allocated_buffer || !aligned_buffer)
		return ffsc (__func__);
	
	void *buffer = mem_alloc (size + alignment);
	if (!buffer)
		return FALSE;
	
	if (clear)
		mem_clear (buffer, size + alignment);
	
	*allocated_buffer = buffer;
	*aligned_buffer = (void *) (((((uintptr_t) buffer) + (uintptr_t) (u64) (alignment - (1 + alignment_offset))) & (uintptr_t) ~((u64) alignment - 1)) + (uintptr_t) (u64) alignment_offset);
	
	return TRUE;
}
