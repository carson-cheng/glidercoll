// Note that _has_neighbours type of functions are preferred over the _bleed_neighbours type, because you can easily get _bleed_neighbours, _on_with_neighbours and _off_with_neighbours
// with an extra operation

// This implementation is slightly faster than summing bits
static always_inline u64 GoLGrid_int_bleed_3_or_more_neighbours_word (u64 upper_word, u64 mid_word, u64 lower_word)
{
	u64 next_cell = (upper_word >> 1);
	u64 sum_at_least_1 = next_cell;
	
	next_cell = upper_word;
	u64 sum_at_least_2 = sum_at_least_1 & next_cell;
	sum_at_least_1 = sum_at_least_1 | next_cell;
	
	next_cell = (upper_word << 1);
	u64 sum_at_least_3 = sum_at_least_2 & next_cell;
	sum_at_least_2 = sum_at_least_2 | (sum_at_least_1 & next_cell);
	sum_at_least_1 = sum_at_least_1 | next_cell;
	
	next_cell = (mid_word >> 1);
	sum_at_least_3 = sum_at_least_3 | (sum_at_least_2 & next_cell);
	sum_at_least_2 = sum_at_least_2 | (sum_at_least_1 & next_cell);
	sum_at_least_1 = sum_at_least_1 | next_cell;
	
	next_cell = (mid_word << 1);
	sum_at_least_3 = sum_at_least_3 | (sum_at_least_2 & next_cell);
	sum_at_least_2 = sum_at_least_2 | (sum_at_least_1 & next_cell);
	sum_at_least_1 = sum_at_least_1 | next_cell;
	
	next_cell = (lower_word >> 1);
	sum_at_least_3 = sum_at_least_3 | (sum_at_least_2 & next_cell);
	sum_at_least_2 = sum_at_least_2 | (sum_at_least_1 & next_cell);
	sum_at_least_1 = sum_at_least_1 | next_cell;
	
	next_cell = lower_word;
	sum_at_least_3 = sum_at_least_3 | (sum_at_least_2 & next_cell);
	sum_at_least_2 = sum_at_least_2 | (sum_at_least_1 & next_cell);
	
	next_cell = (lower_word << 1);
	sum_at_least_3 = sum_at_least_3 | (sum_at_least_2 & next_cell);
	
	return (mid_word | sum_at_least_3);
}

static always_inline u64 GoLGrid_int_bleed_3_or_more_neighbours_column (const u64 *restrict in_entry, u64 *restrict out_entry, i32 row_cnt)
{
	u64 or_of_result = 0;
	i32 row_ix;
	for (row_ix = 0; row_ix < row_cnt; row_ix++)
	{
		u64 out_word = GoLGrid_int_bleed_3_or_more_neighbours_word (in_entry [-1], in_entry [0], in_entry [1]);
		in_entry++;
		
		*out_entry++ = out_word;
		or_of_result |= out_word;
	}
	
	return or_of_result;
}

static always_inline u64 GoLGrid_int_bleed_3_or_more_neighbours_strip (const u64 *restrict in_entry_left, const u64 *restrict in_entry_right, u64 *restrict out_entry_left, u64 *restrict out_entry_right,
		int bit_offset, i32 row_cnt)
{
	u64 or_of_result = 0;
	i32 row_ix;
	for (row_ix = 0; row_ix < row_cnt; row_ix++)
	{
		u64 upper_word = (in_entry_left [-1] << bit_offset) | (in_entry_right [-1] >> (64 - bit_offset));
		u64 mid_word = (in_entry_left [0] << bit_offset) | (in_entry_right [0] >> (64 - bit_offset));
		u64 lower_word = (in_entry_left [1] << bit_offset) | (in_entry_right [1] >> (64 - bit_offset));
		in_entry_left++;
		in_entry_right++;
		
		u64 out_word = GoLGrid_int_bleed_3_or_more_neighbours_word (upper_word, mid_word, lower_word);
		*out_entry_left++ = (out_word >> bit_offset);
		*out_entry_right++ = (out_word << (64 - bit_offset));
		
		or_of_result |= out_word;
	}
	
	return or_of_result;
}

static always_inline u64 GoLGrid_int_bleed_3_or_more_neighbours_column_merge (const u64 *restrict in_entry, u64 *restrict out_entry, i32 row_cnt)
{
	u64 or_of_result = 0;
	i32 row_ix;
	for (row_ix = 0; row_ix < row_cnt; row_ix++)
	{
		u64 out_word = GoLGrid_int_bleed_3_or_more_neighbours_word (in_entry [-1], in_entry [0], in_entry [1]) & 0x7fffffffffffffffu;
		in_entry++;
		
		out_entry [0] = (out_entry [0] & 0x8000000000000000u) | out_word;
		out_entry++;
		
		or_of_result |= out_word;
	}
	
	return or_of_result;
}

static always_inline u64 GoLGrid_int_bleed_3_or_more_neighbours_strip_merge (const u64 *restrict in_entry_left, const u64 *restrict in_entry_right, u64 *restrict out_entry_left, u64 *restrict out_entry_right,
		int bit_offset, i32 row_cnt)
{
	u64 or_of_result = 0;
	i32 row_ix;
	for (row_ix = 0; row_ix < row_cnt; row_ix++)
	{
		u64 upper_word = (in_entry_left [-1] << bit_offset) | (in_entry_right [-1] >> (64 - bit_offset));
		u64 mid_word = (in_entry_left [0] << bit_offset) | (in_entry_right [0] >> (64 - bit_offset));
		u64 lower_word = (in_entry_left [1] << bit_offset) | (in_entry_right [1] >> (64 - bit_offset));
		in_entry_left++;
		in_entry_right++;
		
		u64 out_word = GoLGrid_int_bleed_3_or_more_neighbours_word (upper_word, mid_word, lower_word) & 0x7fffffffffffffffu;
		or_of_result |= out_word;
		
		out_entry_left [0] = (out_entry_left [0] & (((u64) 0xffffffffffffffffu) << (63 - bit_offset))) | (out_word >> bit_offset);
		out_entry_right [0] = out_word << (64 - bit_offset);
		out_entry_left++;
		out_entry_right++;
	}
	
	return or_of_result;
}

static always_inline u64 GoLGrid_int_has_4_or_more_neighbours_word (u64 upper_word, u64 mid_word, u64 lower_word)
{
	u64 nb_0 = upper_word >> 1;
	u64 sum_bit_0 = nb_0;
	
	u64 nb_1 = upper_word;
	u64 sum_bit_1 = sum_bit_0 & nb_1;
	sum_bit_0 = sum_bit_0 ^ nb_1;
	
	u64 nb_2 = upper_word << 1;
	u64 carry_0_to_1 = sum_bit_0 & nb_2;
	sum_bit_1 = sum_bit_1 | carry_0_to_1;
	sum_bit_0 = sum_bit_0 ^ nb_2;
	
	u64 nb_3 = mid_word >> 1;
	carry_0_to_1 = sum_bit_0 & nb_3;
	u64 sum_bit_2_ored_with_bit_3 = sum_bit_1 & carry_0_to_1;
	sum_bit_1 = sum_bit_1 ^ carry_0_to_1;
	sum_bit_0 = sum_bit_0 ^ nb_3;
	
	u64 nb_4 = mid_word << 1;
	carry_0_to_1 = sum_bit_0 & nb_4;
	sum_bit_2_ored_with_bit_3 = sum_bit_2_ored_with_bit_3 | (sum_bit_1 & carry_0_to_1);
	sum_bit_1 = sum_bit_1 ^ carry_0_to_1;
	sum_bit_0 = sum_bit_0 ^ nb_4;
	
	u64 nb_5 = lower_word >> 1;
	u64 side_sum_bit_0 = nb_5;
	
	u64 nb_6 = lower_word;
	u64 side_sum_bit_1 = side_sum_bit_0 & nb_6;
	side_sum_bit_0 = side_sum_bit_0 ^ nb_6;
	
	u64 nb_7 = lower_word << 1;
	carry_0_to_1 = side_sum_bit_0 & nb_7;
	side_sum_bit_1 = side_sum_bit_1 | carry_0_to_1;
	side_sum_bit_0 = side_sum_bit_0 ^ nb_7;
	
	carry_0_to_1 = sum_bit_0 & side_sum_bit_0;
	sum_bit_2_ored_with_bit_3 = sum_bit_2_ored_with_bit_3 | (sum_bit_1 & carry_0_to_1);
	sum_bit_1 = sum_bit_1 ^ carry_0_to_1;
	
	sum_bit_2_ored_with_bit_3 = sum_bit_2_ored_with_bit_3 | (sum_bit_1 & side_sum_bit_1);
	
	return sum_bit_2_ored_with_bit_3;
}

static always_inline u64 GoLGrid_int_has_4_or_more_neighbours_column (const u64 *restrict in_entry, u64 *restrict out_entry, i32 row_cnt)
{
	u64 or_of_result = 0;
	i32 row_ix;
	for (row_ix = 0; row_ix < row_cnt; row_ix++)
	{
		u64 out_word = GoLGrid_int_has_4_or_more_neighbours_word (in_entry [-1], in_entry [0], in_entry [1]);
		in_entry++;
		
		*out_entry++ = out_word;
		or_of_result |= out_word;
	}
	
	return or_of_result;
}

static always_inline u64 GoLGrid_int_has_4_or_more_neighbours_strip (const u64 *restrict in_entry_left, const u64 *restrict in_entry_right, u64 *restrict out_entry_left, u64 *restrict out_entry_right,
		int bit_offset, i32 row_cnt)
{
	u64 or_of_result = 0;
	i32 row_ix;
	for (row_ix = 0; row_ix < row_cnt; row_ix++)
	{
		u64 upper_word = (in_entry_left [-1] << bit_offset) | (in_entry_right [-1] >> (64 - bit_offset));
		u64 mid_word = (in_entry_left [0] << bit_offset) | (in_entry_right [0] >> (64 - bit_offset));
		u64 lower_word = (in_entry_left [1] << bit_offset) | (in_entry_right [1] >> (64 - bit_offset));
		in_entry_left++;
		in_entry_right++;
		
		u64 out_word = GoLGrid_int_has_4_or_more_neighbours_word (upper_word, mid_word, lower_word);
		*out_entry_left++ = (out_word >> bit_offset);
		*out_entry_right++ = (out_word << (64 - bit_offset));
		
		or_of_result |= out_word;
	}
	
	return or_of_result;
}

static always_inline u64 GoLGrid_int_has_4_or_more_neighbours_column_merge (const u64 *restrict in_entry, u64 *restrict out_entry, i32 row_cnt)
{
	u64 or_of_result = 0;
	i32 row_ix;
	for (row_ix = 0; row_ix < row_cnt; row_ix++)
	{
		u64 out_word = GoLGrid_int_has_4_or_more_neighbours_word (in_entry [-1], in_entry [0], in_entry [1]) & 0x7fffffffffffffffu;
		in_entry++;
		
		out_entry [0] = (out_entry [0] & 0x8000000000000000u) | out_word;
		out_entry++;
		
		or_of_result |= out_word;
	}
	
	return or_of_result;
}

static always_inline u64 GoLGrid_int_has_4_or_more_neighbours_strip_merge (const u64 *restrict in_entry_left, const u64 *restrict in_entry_right, u64 *restrict out_entry_left, u64 *restrict out_entry_right,
		int bit_offset, i32 row_cnt)
{
	u64 or_of_result = 0;
	i32 row_ix;
	for (row_ix = 0; row_ix < row_cnt; row_ix++)
	{
		u64 upper_word = (in_entry_left [-1] << bit_offset) | (in_entry_right [-1] >> (64 - bit_offset));
		u64 mid_word = (in_entry_left [0] << bit_offset) | (in_entry_right [0] >> (64 - bit_offset));
		u64 lower_word = (in_entry_left [1] << bit_offset) | (in_entry_right [1] >> (64 - bit_offset));
		in_entry_left++;
		in_entry_right++;
		
		u64 out_word = GoLGrid_int_has_4_or_more_neighbours_word (upper_word, mid_word, lower_word) & 0x7fffffffffffffffu;
		or_of_result |= out_word;
		
		out_entry_left [0] = (out_entry_left [0] & (((u64) 0xffffffffffffffffu) << (63 - bit_offset))) | (out_word >> bit_offset);
		out_entry_right [0] = out_word << (64 - bit_offset);
		out_entry_left++;
		out_entry_right++;
	}
	
	return or_of_result;
}

static always_inline u64 GoLGrid_int_evolves_differently_without_one_neighbour_word (u64 upper_word, u64 mid_word, u64 lower_word)
{
	u64 nb_0 = upper_word >> 1;
	u64 sum_bit_0 = nb_0;
	
	u64 nb_1 = upper_word;
	u64 sum_bit_1 = sum_bit_0 & nb_1;
	sum_bit_0 = sum_bit_0 ^ nb_1;
	
	u64 nb_2 = upper_word << 1;
	u64 carry_0_to_1 = sum_bit_0 & nb_2;
	sum_bit_1 = sum_bit_1 | carry_0_to_1;
	sum_bit_0 = sum_bit_0 ^ nb_2;
	
	u64 nb_3 = mid_word >> 1;
	carry_0_to_1 = sum_bit_0 & nb_3;
	u64 sum_bit_2 = sum_bit_1 & carry_0_to_1;
	sum_bit_1 = sum_bit_1 ^ carry_0_to_1;
	sum_bit_0 = sum_bit_0 ^ nb_3;
	
	u64 nb_4 = mid_word << 1;
	carry_0_to_1 = sum_bit_0 & nb_4;
	sum_bit_2 = sum_bit_2 | (sum_bit_1 & carry_0_to_1);
	sum_bit_1 = sum_bit_1 ^ carry_0_to_1;
	sum_bit_0 = sum_bit_0 ^ nb_4;
	
	u64 nb_5 = lower_word >> 1;
	u64 side_sum_bit_0 = nb_5;
	
	u64 nb_6 = lower_word;
	u64 side_sum_bit_1 = side_sum_bit_0 & nb_6;
	side_sum_bit_0 = side_sum_bit_0 ^ nb_6;
	
	u64 nb_7 = lower_word << 1;
	carry_0_to_1 = side_sum_bit_0 & nb_7;
	side_sum_bit_1 = side_sum_bit_1 | carry_0_to_1;
	side_sum_bit_0 = side_sum_bit_0 ^ nb_7;
	
	carry_0_to_1 = sum_bit_0 & side_sum_bit_0;
	sum_bit_2 = sum_bit_2 | (sum_bit_1 & carry_0_to_1);
	sum_bit_1 = sum_bit_1 ^ carry_0_to_1;
	sum_bit_0 = sum_bit_0 ^ side_sum_bit_0;
	
	u64 carry_1_to_2 = sum_bit_1 & side_sum_bit_1;
	sum_bit_2 = sum_bit_2 ^ carry_1_to_2;
	sum_bit_1 = sum_bit_1 ^ side_sum_bit_1;
	
	return (sum_bit_2 & (~sum_bit_1) & (~sum_bit_0)) | ((~sum_bit_2) & sum_bit_1 & (sum_bit_0 ^ mid_word));
}

static always_inline u64 GoLGrid_int_evolves_differently_without_one_neighbour_column (const u64 *restrict in_entry, u64 *restrict out_entry, i32 row_cnt)
{
	u64 or_of_result = 0;
	i32 row_ix;
	for (row_ix = 0; row_ix < row_cnt; row_ix++)
	{
		u64 out_word = GoLGrid_int_evolves_differently_without_one_neighbour_word (in_entry [-1], in_entry [0], in_entry [1]);
		in_entry++;
		
		*out_entry++ = out_word;
		or_of_result |= out_word;
	}
	
	return or_of_result;
}

// This implementation is slightly faster than the method in bleed_3_or_more_neighbours
static always_inline u64 GoLGrid_int_has_2_neighbours_word (u64 upper_word, u64 mid_word, u64 lower_word)
{
	u64 nb_0 = upper_word >> 1;
	u64 sum_bit_0 = nb_0;
	
	u64 nb_1 = upper_word;
	u64 sum_bit_1 = sum_bit_0 & nb_1;
	sum_bit_0 = sum_bit_0 ^ nb_1;
	
	u64 nb_2 = upper_word << 1;
	u64 carry_0_to_1 = sum_bit_0 & nb_2;
	sum_bit_1 = sum_bit_1 | carry_0_to_1;
	sum_bit_0 = sum_bit_0 ^ nb_2;
	
	u64 nb_3 = mid_word >> 1;
	carry_0_to_1 = sum_bit_0 & nb_3;
	u64 sum_bit_2_ored_with_bit_3 = sum_bit_1 & carry_0_to_1;
	sum_bit_1 = sum_bit_1 ^ carry_0_to_1;
	sum_bit_0 = sum_bit_0 ^ nb_3;
	
	u64 nb_4 = mid_word << 1;
	carry_0_to_1 = sum_bit_0 & nb_4;
	sum_bit_2_ored_with_bit_3 = sum_bit_2_ored_with_bit_3 | (sum_bit_1 & carry_0_to_1);
	sum_bit_1 = sum_bit_1 ^ carry_0_to_1;
	sum_bit_0 = sum_bit_0 ^ nb_4;
	
	u64 nb_5 = lower_word >> 1;
	u64 side_sum_bit_0 = nb_5;
	
	u64 nb_6 = lower_word;
	u64 side_sum_bit_1 = side_sum_bit_0 & nb_6;
	side_sum_bit_0 = side_sum_bit_0 ^ nb_6;
	
	u64 nb_7 = lower_word << 1;
	carry_0_to_1 = side_sum_bit_0 & nb_7;
	side_sum_bit_1 = side_sum_bit_1 | carry_0_to_1;
	side_sum_bit_0 = side_sum_bit_0 ^ nb_7;
	
	carry_0_to_1 = sum_bit_0 & side_sum_bit_0;
	sum_bit_2_ored_with_bit_3 = sum_bit_2_ored_with_bit_3 | (sum_bit_1 & carry_0_to_1);
	sum_bit_1 = sum_bit_1 ^ carry_0_to_1;
	sum_bit_0 = sum_bit_0 ^ side_sum_bit_0;
	
	sum_bit_2_ored_with_bit_3 = sum_bit_2_ored_with_bit_3 | (sum_bit_1 & side_sum_bit_1);
	sum_bit_1 = sum_bit_1 ^ side_sum_bit_1;
	
	return (~sum_bit_2_ored_with_bit_3) & sum_bit_1 & (~sum_bit_0);
}

static always_inline u64 GoLGrid_int_has_2_neighbours_column (const u64 *restrict in_entry, u64 *restrict out_entry, i32 row_cnt)
{
	u64 or_of_result = 0;
	i32 row_ix;
	for (row_ix = 0; row_ix < row_cnt; row_ix++)
	{
		u64 out_word = GoLGrid_int_has_2_neighbours_word (in_entry [-1], in_entry [0], in_entry [1]);
		in_entry++;
		
		*out_entry++ = out_word;
		or_of_result |= out_word;
	}
	
	return or_of_result;
}

static always_inline u64 GoLGrid_int_has_1_or_more_neighbours_word (u64 upper_word, u64 mid_word, u64 lower_word)
{
	return (upper_word >> 1) | upper_word | (upper_word << 1) | (mid_word >> 1) | (mid_word << 1) | (lower_word >> 1) | lower_word | (lower_word << 1);
}

static always_inline u64 GoLGrid_int_has_1_or_more_neighbours_column (const u64 *restrict in_entry, u64 *restrict out_entry, i32 row_cnt)
{
	u64 or_of_result = 0;
	i32 row_ix;
	for (row_ix = 0; row_ix < row_cnt; row_ix++)
	{
		u64 out_word = GoLGrid_int_has_1_or_more_neighbours_word (in_entry [-1], in_entry [0], in_entry [1]);
		in_entry++;
		
		*out_entry++ = out_word;
		or_of_result |= out_word;
	}
	
	return or_of_result;
}

static always_inline void GoLGrid_bleed_3_or_more_neighbours_inline (const GoLGrid *src_gg, GoLGrid *dst_gg)
{
	if (!src_gg || !src_gg->grid || !dst_gg || !dst_gg->grid || dst_gg->grid_rect.width != src_gg->grid_rect.width || dst_gg->grid_rect.height != src_gg->grid_rect.height)
		return (void) ffsc (__func__);
	
	dst_gg->grid_rect.left_x = src_gg->grid_rect.left_x;
	dst_gg->grid_rect.top_y = src_gg->grid_rect.top_y;
	dst_gg->generation = src_gg->generation;
	
	if (src_gg->pop_x_off <= src_gg->pop_x_on)
	{
		GoLGrid_int_clear (dst_gg);
		return;
	}
	
	i32 make_bit_on = higher_of_i32 (src_gg->pop_x_on - 1, 0);
	i32 make_bit_off = lower_of_i32 (src_gg->pop_x_off + 1, src_gg->grid_rect.width);
	i32 make_col_on = make_bit_on >> 6;
	i32 make_col_off = (make_bit_off + 63) >> 6;
	
	i32 required_row_on = higher_of_i32 (src_gg->pop_y_on - 1, 0);
	i32 required_row_off = lower_of_i32 (src_gg->pop_y_off + 1, src_gg->grid_rect.height);
	i32 make_row_on = align_down_i32_twos_compl (required_row_on, TARGET_ARCH_VECTOR_BYTE_SIZE / sizeof (u64));
	i32 make_row_off = align_up_i32_twos_compl (required_row_off, TARGET_ARCH_VECTOR_BYTE_SIZE / sizeof (u64));
	
	i32 make_row_cnt = make_row_off - make_row_on;
	
	if (dst_gg->pop_x_on < dst_gg->pop_x_off)
	{
		i32 clear_col_on = dst_gg->pop_x_on >> 6;
		i32 clear_col_off = (dst_gg->pop_x_off + 63) >> 6;
		
		if (clear_col_on < make_col_on || clear_col_off > make_col_off || dst_gg->pop_y_on < make_row_on || dst_gg->pop_y_off > make_row_off)
			GoLGrid_int_clear_unaffected_area (dst_gg, make_col_on, make_col_off, make_row_on, make_row_off);
	}
	
	u64 col_offset = align_down_u64 (src_gg->col_offset, TARGET_ARCH_VECTOR_BYTE_SIZE / sizeof (u64));
	
	const u64 *in_entry = src_gg->grid + (col_offset * (u64) make_col_on) + (u64) make_row_on;
	u64 *out_entry = dst_gg->grid + (col_offset * (u64) make_col_on) + (u64) make_row_on;
	
	in_entry = align_down_const_pointer (in_entry, TARGET_ARCH_VECTOR_BYTE_SIZE);
	out_entry = align_down_pointer (out_entry, TARGET_ARCH_VECTOR_BYTE_SIZE);
	
	i32 strip_start_bit;
	u64 or_of_result;
	
	if (make_col_off - make_col_on == 1 || (make_bit_on & 0x3f) == 0)
	{
		strip_start_bit = make_bit_on & 0xffffffc0;
		or_of_result = GoLGrid_int_bleed_3_or_more_neighbours_column (in_entry, out_entry, make_row_cnt);
	}
	else
	{
		const u64 *in_entry_right = align_down_const_pointer (in_entry + col_offset, TARGET_ARCH_VECTOR_BYTE_SIZE);
		u64 *out_entry_right = align_down_pointer (out_entry + col_offset, TARGET_ARCH_VECTOR_BYTE_SIZE);
		
		strip_start_bit = make_bit_on;
		or_of_result = GoLGrid_int_bleed_3_or_more_neighbours_strip (in_entry, in_entry_right, out_entry, out_entry_right, strip_start_bit & 0x3f, make_row_cnt);
	}
	
	i32 offset_leftmost_nonempty = strip_start_bit;
	i32 offset_rightmost_nonempty = strip_start_bit;
	i32 next_strip_start_bit = strip_start_bit + 62;
	
	if (next_strip_start_bit < make_bit_off - 2)
		or_of_result &= 0xfffffffffffffffeu;
	
	u64 leftmost_nonempty_or_of_result = or_of_result;
	u64 rightmost_nonempty_or_of_result = or_of_result;
	
	while (next_strip_start_bit < make_bit_off - 2)
	{
		in_entry = align_down_const_pointer (src_gg->grid + (col_offset * (u64) (next_strip_start_bit >> 6)) + (u64) make_row_on, TARGET_ARCH_VECTOR_BYTE_SIZE);
		out_entry = align_down_pointer (dst_gg->grid + (col_offset * (u64) (next_strip_start_bit >> 6)) + (u64) make_row_on, TARGET_ARCH_VECTOR_BYTE_SIZE);
		
		if ((next_strip_start_bit & 0x3f) == 0 || next_strip_start_bit > ((make_col_off - 1) << 6))
		{
			strip_start_bit = next_strip_start_bit & 0xffffffc0;
			or_of_result = GoLGrid_int_bleed_3_or_more_neighbours_column_merge (in_entry, out_entry, make_row_cnt);
		}
		else
		{
			const u64 *in_entry_right = align_down_const_pointer (in_entry + col_offset, TARGET_ARCH_VECTOR_BYTE_SIZE);
			u64 *out_entry_right = align_down_pointer (out_entry + col_offset, TARGET_ARCH_VECTOR_BYTE_SIZE);
			
			strip_start_bit = next_strip_start_bit;
			or_of_result = GoLGrid_int_bleed_3_or_more_neighbours_strip_merge (in_entry, in_entry_right, out_entry, out_entry_right, strip_start_bit & 0x3f, make_row_cnt);
		}
		
		next_strip_start_bit = strip_start_bit + 62;
		
		if (next_strip_start_bit < make_bit_off - 2)
			or_of_result &= 0xfffffffffffffffeu;
		
		if (or_of_result != 0)
		{
			if (leftmost_nonempty_or_of_result == 0)
			{
				leftmost_nonempty_or_of_result = or_of_result;
				offset_leftmost_nonempty = strip_start_bit;
			}
			rightmost_nonempty_or_of_result = or_of_result;
			offset_rightmost_nonempty = strip_start_bit;
		}
	}
	
	if (leftmost_nonempty_or_of_result == 0)
		GoLGrid_int_set_empty_population_rect (dst_gg);
	else
	{
		dst_gg->pop_x_on = offset_leftmost_nonempty + (63 - most_significant_bit_u64_fail_to_64 (leftmost_nonempty_or_of_result));
		dst_gg->pop_x_off = offset_rightmost_nonempty + (64 - least_significant_bit_u64_fail_to_64 (rightmost_nonempty_or_of_result));
		
		dst_gg->pop_y_on = required_row_on;
		dst_gg->pop_y_off = required_row_off;
		GoLGrid_int_tighten_pop_y_on (dst_gg);
		GoLGrid_int_tighten_pop_y_off (dst_gg);
	}
}

static always_inline void GoLGrid_bleed_3_or_more_neighbours_64_wide (const GoLGrid *src_gg, GoLGrid *dst_gg)
{
	if (!src_gg || !src_gg->grid || src_gg->grid_rect.width != 64 || !dst_gg || !dst_gg->grid || dst_gg->grid_rect.width != 64 || dst_gg->grid_rect.height != src_gg->grid_rect.height)
		return (void) ffsc (__func__);
	
	dst_gg->grid_rect.left_x = src_gg->grid_rect.left_x;
	dst_gg->grid_rect.top_y = src_gg->grid_rect.top_y;
	dst_gg->generation = src_gg->generation;
	
	if (src_gg->pop_x_off <= src_gg->pop_x_on)
	{
		GoLGrid_int_clear_64_wide (dst_gg);
		return;
	}
	
	i32 required_row_on = higher_of_i32 (src_gg->pop_y_on - 1, 0);
	i32 required_row_off = lower_of_i32 (src_gg->pop_y_off + 1, src_gg->grid_rect.height);
	i32 make_row_on = align_down_i32_twos_compl (required_row_on, TARGET_ARCH_VECTOR_BYTE_SIZE / sizeof (u64));
	i32 make_row_off = align_up_i32_twos_compl (required_row_off, TARGET_ARCH_VECTOR_BYTE_SIZE / sizeof (u64));
	
	i32 make_row_cnt = make_row_off - make_row_on;
	
	if (dst_gg->pop_x_on < dst_gg->pop_x_off)
		if (dst_gg->pop_y_on < make_row_on || dst_gg->pop_y_off > make_row_off)
			GoLGrid_int_clear_unaffected_area_64_wide (dst_gg, make_row_on, make_row_off);
	
	const u64 *in_entry = align_down_const_pointer (src_gg->grid + (u64) make_row_on, TARGET_ARCH_VECTOR_BYTE_SIZE);
	u64 *out_entry = align_down_pointer (dst_gg->grid + (u64) make_row_on, TARGET_ARCH_VECTOR_BYTE_SIZE);
	
	u64 or_of_result = GoLGrid_int_bleed_3_or_more_neighbours_column (in_entry, out_entry, make_row_cnt);
	
	if (or_of_result == 0)
		GoLGrid_int_set_empty_population_rect (dst_gg);
	else
	{
		dst_gg->pop_x_on = 63 - most_significant_bit_u64_fail_to_64 (or_of_result);
		dst_gg->pop_x_off = 64 - least_significant_bit_u64_fail_to_64 (or_of_result);
		
		dst_gg->pop_y_on = required_row_on;
		dst_gg->pop_y_off = required_row_off;
		GoLGrid_int_tighten_pop_y_on_64_wide (dst_gg);
		GoLGrid_int_tighten_pop_y_off_64_wide (dst_gg);
	}
}

static never_inline void GoLGrid_bleed_3_or_more_neighbours (const GoLGrid *src_gg, GoLGrid *dst_gg)
{
	GoLGrid_bleed_3_or_more_neighbours_inline (src_gg, dst_gg);
}

static always_inline void GoLGrid_bleed_3_or_more_neighbours_opt_64_wide (const GoLGrid *src_gg, GoLGrid *dst_gg)
{
	if (!src_gg)
		return (void) ffsc (__func__);
	
	if (src_gg->grid_rect.width == 64)
		return (void) GoLGrid_bleed_3_or_more_neighbours_64_wide (src_gg, dst_gg);
	else
		return (void) GoLGrid_bleed_3_or_more_neighbours (src_gg, dst_gg);
}

static always_inline void GoLGrid_has_4_or_more_neighbours_inline (const GoLGrid *src_gg, GoLGrid *dst_gg)
{
	if (!src_gg || !src_gg->grid || !dst_gg || !dst_gg->grid || dst_gg->grid_rect.width != src_gg->grid_rect.width || dst_gg->grid_rect.height != src_gg->grid_rect.height)
		return (void) ffsc (__func__);
	
	dst_gg->grid_rect.left_x = src_gg->grid_rect.left_x;
	dst_gg->grid_rect.top_y = src_gg->grid_rect.top_y;
	dst_gg->generation = src_gg->generation;
	
	if (src_gg->pop_x_off <= src_gg->pop_x_on)
	{
		GoLGrid_int_clear (dst_gg);
		return;
	}
	
	// Nothing outside the bounding box of src_gg can have 4 or more neighbours
	i32 make_bit_on = src_gg->pop_x_on;
	i32 make_bit_off = src_gg->pop_x_off;
	i32 make_col_on = make_bit_on >> 6;
	i32 make_col_off = (make_bit_off + 63) >> 6;
	
	i32 required_row_on = src_gg->pop_y_on;
	i32 required_row_off = src_gg->pop_y_off;
	i32 make_row_on = align_down_i32_twos_compl (required_row_on, TARGET_ARCH_VECTOR_BYTE_SIZE / sizeof (u64));
	i32 make_row_off = align_up_i32_twos_compl (required_row_off, TARGET_ARCH_VECTOR_BYTE_SIZE / sizeof (u64));
	
	i32 make_row_cnt = make_row_off - make_row_on;
	
	if (dst_gg->pop_x_on < dst_gg->pop_x_off)
	{
		i32 clear_col_on = dst_gg->pop_x_on >> 6;
		i32 clear_col_off = (dst_gg->pop_x_off + 63) >> 6;
		
		if (clear_col_on < make_col_on || clear_col_off > make_col_off || dst_gg->pop_y_on < make_row_on || dst_gg->pop_y_off > make_row_off)
			GoLGrid_int_clear_unaffected_area (dst_gg, make_col_on, make_col_off, make_row_on, make_row_off);
	}
	
	u64 col_offset = align_down_u64 (src_gg->col_offset, TARGET_ARCH_VECTOR_BYTE_SIZE / sizeof (u64));
	
	const u64 *in_entry = src_gg->grid + (col_offset * (u64) make_col_on) + (u64) make_row_on;
	u64 *out_entry = dst_gg->grid + (col_offset * (u64) make_col_on) + (u64) make_row_on;
	
	in_entry = align_down_const_pointer (in_entry, TARGET_ARCH_VECTOR_BYTE_SIZE);
	out_entry = align_down_pointer (out_entry, TARGET_ARCH_VECTOR_BYTE_SIZE);
	
	i32 strip_start_bit;
	u64 or_of_result;
	
	if (make_col_off - make_col_on == 1 || (make_bit_on & 0x3f) == 0)
	{
		strip_start_bit = make_bit_on & 0xffffffc0;
		or_of_result = GoLGrid_int_has_4_or_more_neighbours_column (in_entry, out_entry, make_row_cnt);
	}
	else
	{
		const u64 *in_entry_right = align_down_const_pointer (in_entry + col_offset, TARGET_ARCH_VECTOR_BYTE_SIZE);
		u64 *out_entry_right = align_down_pointer (out_entry + col_offset, TARGET_ARCH_VECTOR_BYTE_SIZE);
		
		strip_start_bit = make_bit_on;
		or_of_result = GoLGrid_int_has_4_or_more_neighbours_strip (in_entry, in_entry_right, out_entry, out_entry_right, strip_start_bit & 0x3f, make_row_cnt);
	}
	
	i32 offset_leftmost_nonempty = strip_start_bit;
	i32 offset_rightmost_nonempty = strip_start_bit;
	i32 next_strip_start_bit = strip_start_bit + 62;
	
	if (next_strip_start_bit < make_bit_off - 2)
		or_of_result &= 0xfffffffffffffffeu;
	
	u64 leftmost_nonempty_or_of_result = or_of_result;
	u64 rightmost_nonempty_or_of_result = or_of_result;
	
	while (next_strip_start_bit < make_bit_off - 2)
	{
		in_entry = align_down_const_pointer (src_gg->grid + (col_offset * (u64) (next_strip_start_bit >> 6)) + (u64) make_row_on, TARGET_ARCH_VECTOR_BYTE_SIZE);
		out_entry = align_down_pointer (dst_gg->grid + (col_offset * (u64) (next_strip_start_bit >> 6)) + (u64) make_row_on, TARGET_ARCH_VECTOR_BYTE_SIZE);
		
		if ((next_strip_start_bit & 0x3f) == 0 || next_strip_start_bit > ((make_col_off - 1) << 6))
		{
			strip_start_bit = next_strip_start_bit & 0xffffffc0;
			or_of_result = GoLGrid_int_has_4_or_more_neighbours_column_merge (in_entry, out_entry, make_row_cnt);
		}
		else
		{
			const u64 *in_entry_right = align_down_const_pointer (in_entry + col_offset, TARGET_ARCH_VECTOR_BYTE_SIZE);
			u64 *out_entry_right = align_down_pointer (out_entry + col_offset, TARGET_ARCH_VECTOR_BYTE_SIZE);
			
			strip_start_bit = next_strip_start_bit;
			or_of_result = GoLGrid_int_has_4_or_more_neighbours_strip_merge (in_entry, in_entry_right, out_entry, out_entry_right, strip_start_bit & 0x3f, make_row_cnt);
		}
		
		next_strip_start_bit = strip_start_bit + 62;
		
		if (next_strip_start_bit < make_bit_off - 2)
			or_of_result &= 0xfffffffffffffffeu;
		
		if (or_of_result != 0)
		{
			if (leftmost_nonempty_or_of_result == 0)
			{
				leftmost_nonempty_or_of_result = or_of_result;
				offset_leftmost_nonempty = strip_start_bit;
			}
			rightmost_nonempty_or_of_result = or_of_result;
			offset_rightmost_nonempty = strip_start_bit;
		}
	}
	
	if (leftmost_nonempty_or_of_result == 0)
		GoLGrid_int_set_empty_population_rect (dst_gg);
	else
	{
		dst_gg->pop_x_on = offset_leftmost_nonempty + (63 - most_significant_bit_u64_fail_to_64 (leftmost_nonempty_or_of_result));
		dst_gg->pop_x_off = offset_rightmost_nonempty + (64 - least_significant_bit_u64_fail_to_64 (rightmost_nonempty_or_of_result));
		
		dst_gg->pop_y_on = required_row_on;
		dst_gg->pop_y_off = required_row_off;
		GoLGrid_int_tighten_pop_y_on (dst_gg);
		GoLGrid_int_tighten_pop_y_off (dst_gg);
	}
}

static always_inline void GoLGrid_has_4_or_more_neighbours_64_wide (const GoLGrid *src_gg, GoLGrid *dst_gg)
{
	if (!src_gg || !src_gg->grid || src_gg->grid_rect.width != 64 || !dst_gg || !dst_gg->grid || dst_gg->grid_rect.width != 64 || dst_gg->grid_rect.height != src_gg->grid_rect.height)
		return (void) ffsc (__func__);
	
	dst_gg->grid_rect.left_x = src_gg->grid_rect.left_x;
	dst_gg->grid_rect.top_y = src_gg->grid_rect.top_y;
	dst_gg->generation = src_gg->generation;
	
	if (src_gg->pop_x_off <= src_gg->pop_x_on)
	{
		GoLGrid_int_clear_64_wide (dst_gg);
		return;
	}
	
	// Nothing outside the bounding box of src_gg can have 4 or more neighbours
	i32 required_row_on = src_gg->pop_y_on;
	i32 required_row_off = src_gg->pop_y_off;
	i32 make_row_on = align_down_i32_twos_compl (required_row_on, TARGET_ARCH_VECTOR_BYTE_SIZE / sizeof (u64));
	i32 make_row_off = align_up_i32_twos_compl (required_row_off, TARGET_ARCH_VECTOR_BYTE_SIZE / sizeof (u64));
	
	i32 make_row_cnt = make_row_off - make_row_on;
	
	if (dst_gg->pop_x_on < dst_gg->pop_x_off)
		if (dst_gg->pop_y_on < make_row_on || dst_gg->pop_y_off > make_row_off)
			GoLGrid_int_clear_unaffected_area_64_wide (dst_gg, make_row_on, make_row_off);
	
	const u64 *in_entry = align_down_const_pointer (src_gg->grid + (u64) make_row_on, TARGET_ARCH_VECTOR_BYTE_SIZE);
	u64 *out_entry = align_down_pointer (dst_gg->grid + (u64) make_row_on, TARGET_ARCH_VECTOR_BYTE_SIZE);
	
	u64 or_of_result = GoLGrid_int_has_4_or_more_neighbours_column (in_entry, out_entry, make_row_cnt);
	
	if (or_of_result == 0)
		GoLGrid_int_set_empty_population_rect (dst_gg);
	else
	{
		dst_gg->pop_x_on = 63 - most_significant_bit_u64_fail_to_64 (or_of_result);
		dst_gg->pop_x_off = 64 - least_significant_bit_u64_fail_to_64 (or_of_result);
		
		dst_gg->pop_y_on = required_row_on;
		dst_gg->pop_y_off = required_row_off;
		GoLGrid_int_tighten_pop_y_on_64_wide (dst_gg);
		GoLGrid_int_tighten_pop_y_off_64_wide (dst_gg);
	}
}

static never_inline void GoLGrid_has_4_or_more_neighbours (const GoLGrid *src_gg, GoLGrid *dst_gg)
{
	GoLGrid_has_4_or_more_neighbours_inline (src_gg, dst_gg);
}

static always_inline void GoLGrid_has_4_or_more_neighbours_opt_64_wide (const GoLGrid *src_gg, GoLGrid *dst_gg)
{
	if (!src_gg)
		return (void) ffsc (__func__);
	
	if (src_gg->grid_rect.width == 64)
		return (void) GoLGrid_has_4_or_more_neighbours_64_wide (src_gg, dst_gg);
	else
		return (void) GoLGrid_has_4_or_more_neighbours (src_gg, dst_gg);
}

static always_inline void GoLGrid_evolves_differently_without_one_neighbour_64_wide (const GoLGrid *src_gg, GoLGrid *dst_gg)
{
	if (!src_gg || !src_gg->grid || src_gg->grid_rect.width != 64 || !dst_gg || !dst_gg->grid || dst_gg->grid_rect.width != 64 || dst_gg->grid_rect.height != src_gg->grid_rect.height)
		return (void) ffsc (__func__);
	
	dst_gg->grid_rect.left_x = src_gg->grid_rect.left_x;
	dst_gg->grid_rect.top_y = src_gg->grid_rect.top_y;
	dst_gg->generation = src_gg->generation;
	
	if (src_gg->pop_x_off <= src_gg->pop_x_on)
	{
		GoLGrid_int_clear_64_wide (dst_gg);
		return;
	}
	
	i32 required_row_on = higher_of_i32 (src_gg->pop_y_on - 1, 0);
	i32 required_row_off = lower_of_i32 (src_gg->pop_y_off + 1, src_gg->grid_rect.height);
	i32 make_row_on = align_down_i32_twos_compl (required_row_on, TARGET_ARCH_VECTOR_BYTE_SIZE / sizeof (u64));
	i32 make_row_off = align_up_i32_twos_compl (required_row_off, TARGET_ARCH_VECTOR_BYTE_SIZE / sizeof (u64));
	
	i32 make_row_cnt = make_row_off - make_row_on;
	
	if (dst_gg->pop_x_on < dst_gg->pop_x_off)
		if (dst_gg->pop_y_on < make_row_on || dst_gg->pop_y_off > make_row_off)
			GoLGrid_int_clear_unaffected_area_64_wide (dst_gg, make_row_on, make_row_off);
	
	const u64 *in_entry = align_down_const_pointer (src_gg->grid + (u64) make_row_on, TARGET_ARCH_VECTOR_BYTE_SIZE);
	u64 *out_entry = align_down_pointer (dst_gg->grid + (u64) make_row_on, TARGET_ARCH_VECTOR_BYTE_SIZE);
	
	u64 or_of_result = GoLGrid_int_evolves_differently_without_one_neighbour_column (in_entry, out_entry, make_row_cnt);
	
	if (or_of_result == 0)
		GoLGrid_int_set_empty_population_rect (dst_gg);
	else
	{
		dst_gg->pop_x_on = 63 - most_significant_bit_u64_fail_to_64 (or_of_result);
		dst_gg->pop_x_off = 64 - least_significant_bit_u64_fail_to_64 (or_of_result);
		
		dst_gg->pop_y_on = required_row_on;
		dst_gg->pop_y_off = required_row_off;
		GoLGrid_int_tighten_pop_y_on_64_wide (dst_gg);
		GoLGrid_int_tighten_pop_y_off_64_wide (dst_gg);
	}
}

static always_inline void GoLGrid_has_2_neighbours_64_wide (const GoLGrid *src_gg, GoLGrid *dst_gg)
{
	if (!src_gg || !src_gg->grid || src_gg->grid_rect.width != 64 || !dst_gg || !dst_gg->grid || dst_gg->grid_rect.width != 64 || dst_gg->grid_rect.height != src_gg->grid_rect.height)
		return (void) ffsc (__func__);
	
	dst_gg->grid_rect.left_x = src_gg->grid_rect.left_x;
	dst_gg->grid_rect.top_y = src_gg->grid_rect.top_y;
	dst_gg->generation = src_gg->generation;
	
	if (src_gg->pop_x_off <= src_gg->pop_x_on)
	{
		GoLGrid_int_clear_64_wide (dst_gg);
		return;
	}
	
	i32 required_row_on = higher_of_i32 (src_gg->pop_y_on - 1, 0);
	i32 required_row_off = lower_of_i32 (src_gg->pop_y_off + 1, src_gg->grid_rect.height);
	i32 make_row_on = align_down_i32_twos_compl (required_row_on, TARGET_ARCH_VECTOR_BYTE_SIZE / sizeof (u64));
	i32 make_row_off = align_up_i32_twos_compl (required_row_off, TARGET_ARCH_VECTOR_BYTE_SIZE / sizeof (u64));
	
	i32 make_row_cnt = make_row_off - make_row_on;
	
	if (dst_gg->pop_x_on < dst_gg->pop_x_off)
		if (dst_gg->pop_y_on < make_row_on || dst_gg->pop_y_off > make_row_off)
			GoLGrid_int_clear_unaffected_area_64_wide (dst_gg, make_row_on, make_row_off);
	
	const u64 *in_entry = align_down_const_pointer (src_gg->grid + (u64) make_row_on, TARGET_ARCH_VECTOR_BYTE_SIZE);
	u64 *out_entry = align_down_pointer (dst_gg->grid + (u64) make_row_on, TARGET_ARCH_VECTOR_BYTE_SIZE);
	
	u64 or_of_result = GoLGrid_int_has_2_neighbours_column (in_entry, out_entry, make_row_cnt);
	
	if (or_of_result == 0)
		GoLGrid_int_set_empty_population_rect (dst_gg);
	else
	{
		dst_gg->pop_x_on = 63 - most_significant_bit_u64_fail_to_64 (or_of_result);
		dst_gg->pop_x_off = 64 - least_significant_bit_u64_fail_to_64 (or_of_result);
		
		dst_gg->pop_y_on = required_row_on;
		dst_gg->pop_y_off = required_row_off;
		GoLGrid_int_tighten_pop_y_on_64_wide (dst_gg);
		GoLGrid_int_tighten_pop_y_off_64_wide (dst_gg);
	}
}

static always_inline void GoLGrid_has_1_or_more_neighbours_64_wide (const GoLGrid *src_gg, GoLGrid *dst_gg)
{
	if (!src_gg || !src_gg->grid || src_gg->grid_rect.width != 64 || !dst_gg || !dst_gg->grid || dst_gg->grid_rect.width != 64 || dst_gg->grid_rect.height != src_gg->grid_rect.height)
		return (void) ffsc (__func__);
	
	dst_gg->grid_rect.left_x = src_gg->grid_rect.left_x;
	dst_gg->grid_rect.top_y = src_gg->grid_rect.top_y;
	dst_gg->generation = src_gg->generation;
	
	if (src_gg->pop_x_off <= src_gg->pop_x_on)
	{
		GoLGrid_int_clear_64_wide (dst_gg);
		return;
	}
	
	i32 required_row_on = higher_of_i32 (src_gg->pop_y_on - 1, 0);
	i32 required_row_off = lower_of_i32 (src_gg->pop_y_off + 1, src_gg->grid_rect.height);
	i32 make_row_on = align_down_i32_twos_compl (required_row_on, TARGET_ARCH_VECTOR_BYTE_SIZE / sizeof (u64));
	i32 make_row_off = align_up_i32_twos_compl (required_row_off, TARGET_ARCH_VECTOR_BYTE_SIZE / sizeof (u64));
	
	i32 make_row_cnt = make_row_off - make_row_on;
	
	if (dst_gg->pop_x_on < dst_gg->pop_x_off)
		if (dst_gg->pop_y_on < make_row_on || dst_gg->pop_y_off > make_row_off)
			GoLGrid_int_clear_unaffected_area_64_wide (dst_gg, make_row_on, make_row_off);
	
	const u64 *in_entry = align_down_const_pointer (src_gg->grid + (u64) make_row_on, TARGET_ARCH_VECTOR_BYTE_SIZE);
	u64 *out_entry = align_down_pointer (dst_gg->grid + (u64) make_row_on, TARGET_ARCH_VECTOR_BYTE_SIZE);
	
	u64 or_of_result = GoLGrid_int_has_1_or_more_neighbours_column (in_entry, out_entry, make_row_cnt);
	
	if (or_of_result == 0)
		GoLGrid_int_set_empty_population_rect (dst_gg);
	else
	{
		dst_gg->pop_x_on = 63 - most_significant_bit_u64_fail_to_64 (or_of_result);
		dst_gg->pop_x_off = 64 - least_significant_bit_u64_fail_to_64 (or_of_result);
		
		dst_gg->pop_y_on = required_row_on;
		dst_gg->pop_y_off = required_row_off;
		GoLGrid_int_tighten_pop_y_on_64_wide (dst_gg);
		GoLGrid_int_tighten_pop_y_off_64_wide (dst_gg);
	}
}

static always_inline void GoLGrid_evolve_single_column (const GoLGrid *in_gg, GoLGrid *out_gg)
{
	if (!in_gg || !in_gg->grid || ((in_gg->pop_x_on < in_gg->pop_x_off) && (1 + (in_gg->pop_x_off >> 6) > ((in_gg->pop_x_on + 63) >> 6))) ||
			!out_gg || !out_gg->grid || out_gg->grid_rect.width != in_gg->grid_rect.width || out_gg->grid_rect.height != in_gg->grid_rect.height)
		return (void) ffsc (__func__);
	
	out_gg->grid_rect.left_x = in_gg->grid_rect.left_x;
	out_gg->grid_rect.top_y = in_gg->grid_rect.top_y;
	out_gg->generation = in_gg->generation + 1;
	
	if (in_gg->pop_x_off <= in_gg->pop_x_on)
	{
		GoLGrid_int_clear (out_gg);
		return;
	}
	
	i32 make_col_on = in_gg->pop_x_on >> 6;
	i32 make_col_off = make_col_on + 1;
	
	i32 required_row_on = higher_of_i32 (in_gg->pop_y_on - 1, 0);
	i32 required_row_off = lower_of_i32 (in_gg->pop_y_off + 1, in_gg->grid_rect.height);
	i32 make_row_on = align_down_i32_twos_compl (required_row_on, TARGET_ARCH_VECTOR_BYTE_SIZE / sizeof (u64));
	i32 make_row_off = align_up_i32_twos_compl (required_row_off, TARGET_ARCH_VECTOR_BYTE_SIZE / sizeof (u64));
	
	i32 make_row_cnt = make_row_off - make_row_on;
	
	if (out_gg->pop_x_on < out_gg->pop_x_off)
	{
		i32 clear_col_on = out_gg->pop_x_on >> 6;
		i32 clear_col_off = (out_gg->pop_x_off + 63) >> 6;
		
		if (clear_col_on < make_col_on || clear_col_off > make_col_off || out_gg->pop_y_on < make_row_on || out_gg->pop_y_off > make_row_off)
			GoLGrid_int_clear_unaffected_area (out_gg, make_col_on, make_col_off, make_row_on, make_row_off);
	}
	
	u64 col_offset = align_down_u64 (in_gg->col_offset, TARGET_ARCH_VECTOR_BYTE_SIZE / sizeof (u64));
	
	const u64 *in_entry = in_gg->grid + (col_offset * (u64) make_col_on) + (u64) make_row_on;
	u64 *out_entry = out_gg->grid + (col_offset * (u64) make_col_on) + (u64) make_row_on;
	
	in_entry = align_down_const_pointer (in_entry, TARGET_ARCH_VECTOR_BYTE_SIZE);
	out_entry = align_down_pointer (out_entry, TARGET_ARCH_VECTOR_BYTE_SIZE);
	
	u64 or_of_result = GoLGrid_int_evolve_column (in_entry, out_entry, make_row_cnt);
	
	if (or_of_result == 0)
		GoLGrid_int_set_empty_population_rect (out_gg);
	else
	{
		out_gg->pop_x_on = (64 * make_col_on) + (63 - most_significant_bit_u64_fail_to_64 (or_of_result));
		out_gg->pop_x_off = (64 * make_col_on) + (64 - least_significant_bit_u64_fail_to_64 (or_of_result));
		
		out_gg->pop_y_on = required_row_on;
		out_gg->pop_y_off = required_row_off;
		GoLGrid_int_tighten_pop_y_on (out_gg);
		GoLGrid_int_tighten_pop_y_off (out_gg);
	}
}

static always_inline void GoLGrid_evolve_opt_single_column (const GoLGrid *in_gg, GoLGrid *out_gg)
{
	if (!in_gg)
		return (void) ffsc (__func__);

// FIXME: This condition is too restrictive for edge columns
	if (1 + (in_gg->pop_x_off >> 6) == ((in_gg->pop_x_on + 63) >> 6))
		GoLGrid_evolve_single_column (in_gg, out_gg);
	else
		GoLGrid_evolve (in_gg, out_gg);
}
