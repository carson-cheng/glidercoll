static always_inline int parse_u32_int (const char *str_in, int wrap, u32 fail_value, u32 *result, int *overflow, const char **str_out, i64 *parse_size)
{
	if (result)
		*result = fail_value;
	if (overflow)
		*overflow = FALSE;
	if (str_out)
		*str_out = str_in;
	if (parse_size)
		*parse_size = 0;
	
	if (!str_in)
		return ffsc (__func__);
	
	i64 size = 0;
	int started = FALSE;
	int oflow = FALSE;
	u32 n = 0;
	while (TRUE)
	{
		u32 c = (u32) (u8) *str_in;
		if ((c < (u32) (u8) '0') || (c > (u32) (u8) '9'))
			break;
		
		str_in++;
		size++;
		started = TRUE;
		
		u32 d = c - (u32) (u8) '0';
		if (n > (u32_MAX / (u32) 10) || (n == (u32_MAX / (u32) 10) && d > (u32_MAX % (u32) 10)))
			oflow = TRUE;
		
		n = (u32) 10 * n + d;
	}
	
	if (overflow && oflow)
		*overflow = TRUE;
	if (str_out)
		*str_out = str_in;
	if (parse_size)
		*parse_size = size;
	
	if (!started || (oflow && !wrap))
		n = fail_value;
	if (result)
		*result = n;
	
	return (started && !oflow);
}

static never_inline u32 get_u32 (const char *str)
{
	u32 result;
	parse_u32_int (str, FALSE, 0, &result, NULL, NULL, NULL);
	return result;
}

static never_inline int str_to_u32 (const char *str, u32 *result)
{
	if (!result)
		return ffsc (__func__);
	
	return parse_u32_int (str, FALSE, 0, result, NULL, NULL, NULL);
}

static never_inline int parse_u32 (const char **str, u32 *result, int *overflow)
{
	if (!str)
		return ffsc (__func__);
	
	const char *s = *str;
	return parse_u32_int (s, FALSE, 0, result, overflow, str, NULL);
}

static never_inline int parse_u32_full (const char **str, int wrap, u32 fail_value, u32 *result, int *overflow, i64 *parse_size)
{
	if (!str)
		return ffsc (__func__);
	
	const char *s = *str;
	return parse_u32_int (s, wrap, fail_value, result, overflow, str, parse_size);
}

static always_inline int parse_i32_int (const char *str_in, int allow_unary_plus, i32 fail_value, i32 *result, int *overflow, const char **str_out, i64 *parse_size)
{
	if (result)
		*result = fail_value;
	if (overflow)
		*overflow = FALSE;
	if (str_out)
		*str_out = str_in;
	if (parse_size)
		*parse_size = 0;
	
	if (!str_in)
		return ffsc (__func__);
	
	i64 size = 0;
	int negative = FALSE;
	if (*str_in == '-')
	{
		str_in++;
		size++;
		negative = TRUE;
	}
	else if (allow_unary_plus && (*str_in == '+'))
	{
		str_in++;
		size++;
	}
	
	int started = FALSE;
	int oflow = FALSE;
	u32 n = 0;
	while (TRUE)
	{
		u32 c = (u32) (u8) *str_in;
		if ((c < (u32) (u8) '0') || (c > (u32) (u8) '9'))
			break;
		
		str_in++;
		size++;
		started = TRUE;
		if (!oflow)
		{
			u32 d = c - (u32) (u8) '0';
			if (n > ((u32) i32_MAX / (u32) 10) || (n == ((u32) i32_MAX / (u32) 10) && d > ((u32) ABS_i32_MIN % (u32) 10)))
				oflow = TRUE;
			
			n = (u32) 10 * n + d;
		}
	}
	
	if (!oflow && !negative && n == (u32) ABS_i32_MIN)
		oflow = TRUE;
	
	if (overflow && oflow)
		*overflow = TRUE;
	if (str_out)
		*str_out = str_in;
	if (parse_size)
		*parse_size = size;
	
	if (!started || oflow)
		return FALSE;
	
	if (result)
	{
		if (n == (u32) ABS_i32_MIN)
			*result = i32_MIN;
		else if (negative)
			*result = -(i32) n;
		else
			*result = (i32) n;
	}
	
	return TRUE;
}

static never_inline i32 get_i32 (const char *str)
{
	i32 result;
	parse_i32_int (str, FALSE, 0, &result, NULL, NULL, NULL);
	return result;
}

static never_inline int str_to_i32 (const char *str, i32 *result)
{
	if (!result)
		return ffsc (__func__);
	
	return parse_i32_int (str, FALSE, 0, result, NULL, NULL, NULL);
}

static never_inline int parse_i32 (const char **str, i32 *result, int *overflow)
{
	if (!str)
		return ffsc (__func__);
	
	const char *s = *str;
	return parse_i32_int (s, FALSE, 0, result, overflow, str, NULL);
}

static never_inline int parse_i32_full (const char **str, int allow_unary_plus, i32 fail_value, i32 *result, int *overflow, i64 *parse_size)
{
	if (!str)
		return ffsc (__func__);
	
	const char *s = *str;
	return parse_i32_int (s, allow_unary_plus, fail_value, result, overflow, str, parse_size);
}

static always_inline int parse_u64_int (const char *str_in, int wrap, u64 fail_value, u64 *result, int *overflow, const char **str_out, i64 *parse_size)
{
	if (result)
		*result = fail_value;
	if (overflow)
		*overflow = FALSE;
	if (str_out)
		*str_out = str_in;
	if (parse_size)
		*parse_size = 0;
	
	if (!str_in)
		return ffsc (__func__);
	
	i64 size = 0;
	int started = FALSE;
	int oflow = FALSE;
	u64 n = 0;
	while (TRUE)
	{
		u64 c = (u64) (u8) *str_in;
		if ((c < (u64) (u8) '0') || (c > (u64) (u8) '9'))
			break;
		
		str_in++;
		size++;
		started = TRUE;
		
		u64 d = c - (u64) (u8) '0';
		if (n > (u64_MAX / (u64) 10) || (n == (u64_MAX / (u64) 10) && d > (u64_MAX % (u64) 10)))
			oflow = TRUE;
		
		n = (u64) 10 * n + d;
	}
	
	if (overflow && oflow)
		*overflow = TRUE;
	if (str_out)
		*str_out = str_in;
	if (parse_size)
		*parse_size = size;
	
	if (!started || (oflow && !wrap))
		n = fail_value;
	if (result)
		*result = n;
	
	return (started && !oflow);
}

static never_inline u64 get_u64 (const char *str)
{
	u64 result;
	parse_u64_int (str, FALSE, 0, &result, NULL, NULL, NULL);
	return result;
}

static never_inline int str_to_u64 (const char *str, u64 *result)
{
	if (!result)
		return ffsc (__func__);
	
	return parse_u64_int (str, FALSE, 0, result, NULL, NULL, NULL);
}

static never_inline int parse_u64 (const char **str, u64 *result, int *overflow)
{
	if (!str)
		return ffsc (__func__);
	
	const char *s = *str;
	return parse_u64_int (s, FALSE, 0, result, overflow, str, NULL);
}

static never_inline int parse_u64_full (const char **str, int wrap, u64 fail_value, u64 *result, int *overflow, i64 *parse_size)
{
	if (!str)
		return ffsc (__func__);
	
	const char *s = *str;
	return parse_u64_int (s, wrap, fail_value, result, overflow, str, parse_size);
}

static always_inline int parse_i64_int (const char *str_in, int allow_unary_plus, i64 fail_value, i64 *result, int *overflow, const char **str_out, i64 *parse_size)
{
	if (result)
		*result = fail_value;
	if (overflow)
		*overflow = FALSE;
	if (str_out)
		*str_out = str_in;
	if (parse_size)
		*parse_size = 0;
	
	if (!str_in)
		return ffsc (__func__);
	
	i64 size = 0;
	int negative = FALSE;
	if (*str_in == '-')
	{
		str_in++;
		size++;
		negative = TRUE;
	}
	else if (allow_unary_plus && (*str_in == '+'))
	{
		str_in++;
		size++;
	}
	
	int started = FALSE;
	int oflow = FALSE;
	u64 n = 0;
	while (TRUE)
	{
		u64 c = (u64) (u8) *str_in;
		if ((c < (u64) (u8) '0') || (c > (u64) (u8) '9'))
			break;
		
		str_in++;
		size++;
		started = TRUE;
		if (!oflow)
		{
			u64 d = c - (u64) (u8) '0';
			if (n > ((u64) i64_MAX / (u64) 10) || (n == ((u64) i64_MAX / (u64) 10) && d > ((u64) ABS_i64_MIN % (u64) 10)))
				oflow = TRUE;
			
			n = (u64) 10 * n + d;
		}
	}
	
	if (!oflow && !negative && n == (u64) ABS_i64_MIN)
		oflow = TRUE;
	
	if (overflow && oflow)
		*overflow = TRUE;
	if (str_out)
		*str_out = str_in;
	if (parse_size)
		*parse_size = size;
	
	if (!started || oflow)
		return FALSE;
	
	if (result)
	{
		if (n == (u64) ABS_i64_MIN)
			*result = i64_MIN;
		else if (negative)
			*result = -(i64) n;
		else
			*result = (i64) n;
	}
	
	return TRUE;
}

static never_inline i64 get_i64 (const char *str)
{
	i64 result;
	parse_i64_int (str, FALSE, 0, &result, NULL, NULL, NULL);
	return result;
}

static never_inline int str_to_i64 (const char *str, i64 *result)
{
	if (!result)
		return ffsc (__func__);
	
	return parse_i64_int (str, FALSE, 0, result, NULL, NULL, NULL);
}

static never_inline int parse_i64 (const char **str, i64 *result, int *overflow)
{
	if (!str)
		return ffsc (__func__);
	
	const char *s = *str;
	return parse_i64_int (s, FALSE, 0, result, overflow, str, NULL);
}

static never_inline int parse_i64_full (const char **str, int allow_unary_plus, i64 fail_value, i64 *result, int *overflow, i64 *parse_size)
{
	if (!str)
		return ffsc (__func__);
	
	const char *s = *str;
	return parse_i64_int (s, allow_unary_plus, fail_value, result, overflow, str, parse_size);
}

static always_inline int digits_in_u64 (u64 op)
{
	static const u64 op_off_of_digit_cnt [20] =
		{0ull, 10ull, 100ull, 1000ull, 10000ull, 100000ull, 1000000ull, 10000000ull, 100000000ull, 1000000000ull, 10000000000ull,
		 100000000000ull, 1000000000000ull, 10000000000000ull, 100000000000000ull, 1000000000000000ull, 10000000000000000ull,
		 100000000000000000ull, 1000000000000000000ull, 10000000000000000000ull};

	int digit_cnt = (int) ((u32) (((i32) most_significant_bit_u64_fail_to_m1 (op) * 77) + 256) >> 8);
	digit_cnt += (op_off_of_digit_cnt [digit_cnt] > op ? 0 : 1);
	return digit_cnt;
}

static always_inline int digits_in_u32 (u32 op)
{
	return digits_in_u64 ((u64) op);
}

static always_inline int digits_in_i32 (i32 op)
{
	int digit_cnt = digits_in_u64 ((u64) abs_i32_to_u32 (op));
	return digit_cnt + (op < 0 ? 1 : 0);
}

static always_inline int digits_in_i64 (i64 op)
{
	int digit_cnt = digits_in_u64 (abs_i64_to_u64 (op));
	return digit_cnt + (op < 0 ? 1 : 0);
}

static always_inline int parse_u32_hex_int (const char *str_in, int wrap, u32 fail_value, u32 *result, int *overflow, const char **str_out, i64 *parse_size)
{
	if (result)
		*result = fail_value;
	if (overflow)
		*overflow = FALSE;
	if (str_out)
		*str_out = str_in;
	if (parse_size)
		*parse_size = 0;
	
	if (!str_in)
		return ffsc (__func__);
	
	i64 size = 0;
	int started = FALSE;
	int oflow = FALSE;
	u32 n = 0;
	while (TRUE)
	{
		u32 c = (u32) (u8) *str_in;
		u32 d;
		if ((c >= (u32) (u8) '0') && (c <= (u32) (u8) '9'))
			d = c - (u32) (u8) '0';
		else if ((c >= (u32) (u8) 'A') && (c <= (u32) (u8) 'F'))
			d = (u32) 10 + (c - (u32) (u8) 'A');
		else if ((c >= (u32) (u8) 'a') && (c <= (u32) (u8) 'f'))
			d = (u32) 10 + (c - (u32) (u8) 'a');
		else
			break;
		
		str_in++;
		size++;
		started = TRUE;
		
		if (n > (u32_MAX / (u32) 16))
			oflow = TRUE;
		
		n = (u32) 16 * n + d;
	}
	
	if (overflow && oflow)
		*overflow = TRUE;
	if (str_out)
		*str_out = str_in;
	if (parse_size)
		*parse_size = size;
	
	if (!started || (oflow && !wrap))
		n = fail_value;
	if (result)
		*result = n;
	
	return (started && !oflow);
}

static never_inline u32 get_u32_hex (const char *str)
{
	u32 result;
	parse_u32_hex_int (str, FALSE, 0, &result, NULL, NULL, NULL);
	return result;
}

static never_inline int str_to_u32_hex (const char *str, u32 *result)
{
	if (!result)
		return ffsc (__func__);
	
	return parse_u32_hex_int (str, FALSE, 0, result, NULL, NULL, NULL);
}

static never_inline int parse_u32_hex (const char **str, u32 *result, int *overflow)
{
	if (!str)
		return ffsc (__func__);
	
	const char *s = *str;
	return parse_u32_hex_int (s, FALSE, 0, result, overflow, str, NULL);
}

static never_inline int parse_u32_hex_full (const char **str, int wrap, u32 fail_value, u32 *result, int *overflow, i64 *parse_size)
{
	if (!str)
		return ffsc (__func__);
	
	const char *s = *str;
	return parse_u32_hex_int (s, wrap, fail_value, result, overflow, str, parse_size);
}

static always_inline int parse_u64_hex_int (const char *str_in, int wrap, u64 fail_value, u64 *result, int *overflow, const char **str_out, i64 *parse_size)
{
	if (result)
		*result = fail_value;
	if (overflow)
		*overflow = FALSE;
	if (str_out)
		*str_out = str_in;
	if (parse_size)
		*parse_size = 0;
	
	if (!str_in)
		return ffsc (__func__);
	
	i64 size = 0;
	int started = FALSE;
	int oflow = FALSE;
	u64 n = 0;
	while (TRUE)
	{
		u64 c = (u64) (u8) *str_in;
		u64 d;
		if ((c >= (u64) (u8) '0') && (c <= (u64) (u8) '9'))
			d = c - (u64) (u8) '0';
		else if ((c >= (u64) (u8) 'A') && (c <= (u64) (u8) 'F'))
			d = (u64) 10 + (c - (u64) (u8) 'A');
		else if ((c >= (u64) (u8) 'a') && (c <= (u64) (u8) 'f'))
			d = (u64) 10 + (c - (u64) (u8) 'a');
		else
			break;
		
		str_in++;
		size++;
		started = TRUE;
		
		if (n > (u64_MAX / (u64) 16))
			oflow = TRUE;
		
		n = (u64) 16 * n + d;
	}
	
	if (overflow && oflow)
		*overflow = TRUE;
	if (str_out)
		*str_out = str_in;
	if (parse_size)
		*parse_size = size;
	
	if (!started || (oflow && !wrap))
		n = fail_value;
	if (result)
		*result = n;
	
	return (started && !oflow);
}

static never_inline u64 get_u64_hex (const char *str)
{
	u64 result;
	parse_u64_hex_int (str, FALSE, 0, &result, NULL, NULL, NULL);
	return result;
}

static never_inline int str_to_u64_hex (const char *str, u64 *result)
{
	if (!result)
		return ffsc (__func__);
	
	return parse_u64_hex_int (str, FALSE, 0, result, NULL, NULL, NULL);
}

static never_inline int parse_u64_hex (const char **str, u64 *result, int *overflow)
{
	if (!str)
		return ffsc (__func__);
	
	const char *s = *str;
	return parse_u64_hex_int (s, FALSE, 0, result, overflow, str, NULL);
}

static never_inline int parse_u64_hex_full (const char **str, int wrap, u64 fail_value, u64 *result, int *overflow, i64 *parse_size)
{
	if (!str)
		return ffsc (__func__);
	
	const char *s = *str;
	return parse_u64_hex_int (s, wrap, fail_value, result, overflow, str, parse_size);
}

static always_inline int parse_u32_bin_int (const char *str_in, int wrap, u32 fail_value, u32 *result, int *overflow, const char **str_out, i64 *parse_size)
{
	if (result)
		*result = fail_value;
	if (overflow)
		*overflow = FALSE;
	if (str_out)
		*str_out = str_in;
	if (parse_size)
		*parse_size = 0;
	
	if (!str_in)
		return ffsc (__func__);
	
	i64 size = 0;
	int started = FALSE;
	int oflow = FALSE;
	u32 n = 0;
	while (TRUE)
	{
		u32 c = (u32) (u8) *str_in;
		if ((c < (u32) (u8) '0') || (c > (u32) (u8) '1'))
			break;
		
		str_in++;
		size++;
		started = TRUE;
		
		u32 d = c - (u32) (u8) '0';
		if (n > (u32_MAX / (u32) 2))
			oflow = TRUE;
		
		n = (u32) 2 * n + d;
	}
	
	if (overflow && oflow)
		*overflow = TRUE;
	if (str_out)
		*str_out = str_in;
	if (parse_size)
		*parse_size = size;
	
	if (!started || (oflow && !wrap))
		n = fail_value;
	if (result)
		*result = n;
	
	return (started && !oflow);
}

static never_inline u32 get_u32_bin (const char *str)
{
	u32 result;
	parse_u32_bin_int (str, FALSE, 0, &result, NULL, NULL, NULL);
	return result;
}

static never_inline int str_to_u32_bin (const char *str, u32 *result)
{
	if (!result)
		return ffsc (__func__);
	
	return parse_u32_bin_int (str, FALSE, 0, result, NULL, NULL, NULL);
}

static never_inline int parse_u32_bin (const char **str, u32 *result, int *overflow)
{
	if (!str)
		return ffsc (__func__);
	
	const char *s = *str;
	return parse_u32_bin_int (s, FALSE, 0, result, overflow, str, NULL);
}

static never_inline int parse_u32_bin_full (const char **str, int wrap, u32 fail_value, u32 *result, int *overflow, i64 *parse_size)
{
	if (!str)
		return ffsc (__func__);
	
	const char *s = *str;
	return parse_u32_bin_int (s, wrap, fail_value, result, overflow, str, parse_size);
}

static always_inline int parse_u64_bin_int (const char *str_in, int wrap, u64 fail_value, u64 *result, int *overflow, const char **str_out, i64 *parse_size)
{
	if (result)
		*result = fail_value;
	if (overflow)
		*overflow = FALSE;
	if (str_out)
		*str_out = str_in;
	if (parse_size)
		*parse_size = 0;
	
	if (!str_in)
		return ffsc (__func__);
	
	i64 size = 0;
	int started = FALSE;
	int oflow = FALSE;
	u64 n = 0;
	while (TRUE)
	{
		u64 c = (u64) (u8) *str_in;
		if ((c < (u64) (u8) '0') || (c > (u64) (u8) '1'))
			break;
		
		str_in++;
		size++;
		started = TRUE;
		
		u64 d = c - (u64) (u8) '0';
		if (n > (u64_MAX / (u64) 2))
			oflow = TRUE;
		
		n = (u64) 2 * n + d;
	}
	
	if (overflow && oflow)
		*overflow = TRUE;
	if (str_out)
		*str_out = str_in;
	if (parse_size)
		*parse_size = size;
	
	if (!started || (oflow && !wrap))
		n = fail_value;
	if (result)
		*result = n;
	
	return (started && !oflow);
}

static never_inline u64 get_u64_bin (const char *str)
{
	u64 result;
	parse_u64_bin_int (str, FALSE, 0, &result, NULL, NULL, NULL);
	return result;
}

static never_inline int str_to_u64_bin (const char *str, u64 *result)
{
	if (!result)
		return ffsc (__func__);
	
	return parse_u64_bin_int (str, FALSE, 0, result, NULL, NULL, NULL);
}

static never_inline int parse_u64_bin (const char **str, u64 *result, int *overflow)
{
	if (!str)
		return ffsc (__func__);
	
	const char *s = *str;
	return parse_u64_bin_int (s, FALSE, 0, result, overflow, str, NULL);
}

static never_inline int parse_u64_bin_full (const char **str, int wrap, u64 fail_value, u64 *result, int *overflow, i64 *parse_size)
{
	if (!str)
		return ffsc (__func__);
	
	const char *s = *str;
	return parse_u64_bin_int (s, wrap, fail_value, result, overflow, str, parse_size);
}
