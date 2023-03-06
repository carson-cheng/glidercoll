static always_inline u64 GoLGrid_int_try_canonical (const GoLGrid *src_gg, u64 *lowest_hash, const RandomDataArray *rda, GoLGrid *dst_gg)
{
	u64 hash = GoLGrid_get_hash (src_gg, rda);
	
	if (!lowest_hash)
	{
		if (dst_gg)
			GoLGrid_copy (src_gg, dst_gg);
		
		return hash;
	}
	else if (hash < *lowest_hash)
	{
		*lowest_hash = hash;
		if (dst_gg)
			GoLGrid_copy (src_gg, dst_gg);
	}
	
	return *lowest_hash;
}

static never_inline void GoLGrid_int_print_life_history_symbol (FILE *stream, i32 state, int use_life_symbols, i32 count, i32 max_line_length, i32 *line_length)
{
	if (!stream || !line_length)
		return (void) ffsc (__func__);
	
	if (count == 0)
		return;
	
	char symbol = '?';
	if (state == -1)
		symbol = '$';
	else if (state == 0)
		symbol = (use_life_symbols ? 'b' : '.');
	else if (state == 1)
		symbol = (use_life_symbols ? 'o' : 'A');
	else 
		symbol = 'A' + (state - 1);
	
	if (count == 1)
	{
		fprintf (stream, "%c", symbol);
		(*line_length)++;
	}
	else if (count > 1)
	{
		fprintf (stream, "%d%c", count, symbol);
		(*line_length) += (1 + digits_in_u32 ((u32) count));
	}
	
	if (max_line_length > 0 && *line_length > max_line_length)
	{
		fprintf (stream, "\n");
		(*line_length) = 0;
	}
}

static never_inline void GoLGrid_int_print_life_history_include_rect (Rect *print_rect, const GoLGrid *gg)
{
	if (!gg)
		return;
	
	if (print_rect->width == 0 || print_rect->height == 0)
		Rect_copy (&gg->grid_rect, print_rect);
	else
		Rect_union (print_rect, &gg->grid_rect, print_rect);
}

static never_inline int GoLGrid_int_get_life_history_symbol (const char **lh, int *success, int *state, i32 *count)
{
	if (success)
		*success = FALSE;
	if (state)
		*state = 0;
	if (count)
		*count = 0;
	
	if (!lh || !*lh || !success || !state || !count)
		return ffsc (__func__);
	
	while (TRUE)
	{
		char c = **lh;
		
		if (c == '#' || c == 'x')
		{
			while (TRUE)
			{
				(*lh)++;
				c = **lh;
				if (c == '\n' || c == '\r' || c == '\0')
					break;
			}
			continue;
		}
		
		if (c == '!' || c == '\0')
		{
			*success = TRUE;
			return FALSE;
		}
		
		if (c == ' ' || c == '\t' || c == '\n' || c == '\r')
		{
			(*lh)++;
			continue;
		}
		
		i32 cnt = 1;
		if (c >= '0' && c <= '9')
		{
			u64 cnt_u64;
			if (!parse_u64 (lh, &cnt_u64, NULL) || cnt_u64 > i32_MAX)
				return FALSE;
			
			cnt = cnt_u64;
			c = **lh;
		}
		
		if (c == '$')
			*state = -1;
		else if (c == '.' || c == 'b')
			*state = 0;
		else if (c == 'o')
			*state = 1;
		else if (c >= 'A' && c <= 'F')
			*state = 1 + (c - 'A');
		else
			return FALSE;
		
		(*lh)++;
		*success = TRUE;
		*count = cnt;
		return TRUE;
	}
}

static never_inline GoLGrid *GoLGrid_alloc_heap_obj (i32 left_x, i32 top_y, i32 width, i32 height)
{
	GoLGrid *gg = malloc (sizeof (GoLGrid));
	if (!gg)
	{
		fprintf (stderr, "Out of memory allocating GoLGrid object");
		return NULL;
	}
	
	Rect grid_rect;
	Rect_make (&grid_rect, left_x, top_y, width, height);
	
	if (!GoLGrid_create (gg, &grid_rect))
	{
		free (gg);
		return NULL;
	}
	
	return gg;
}

static never_inline void GoLGrid_free_heap_obj (GoLGrid **gg)
{
	if (!gg || !*gg)
		return (void) ffsc (__func__);
	
	GoLGrid_free (*gg);
	free (*gg);
	
	*gg = NULL;
}

static never_inline int GoLGrid_or_glider (GoLGrid *gg, const Glider *glider, int consider_gg_generation)
{
	if (!gg || !glider)
		return ffsc (__func__);
	
	Glider adj_glider;
	const Glider *glider_to_use = glider;
	
	if (consider_gg_generation)
	{
		adj_glider.dir = glider->dir;
		adj_glider.lane = glider->lane;
		adj_glider.timing = glider->timing - gg->generation;
		
		glider_to_use = &adj_glider;
	}
	
	ObjCellList ocl;
	Objects_make_glider_obj_cell_list (&ocl, glider_to_use);
	
	return GoLGrid_or_obj_cell_list_inline (gg, &ocl, 0, 0);
}

static never_inline i32 GoLGrid_get_safe_glider_progression (const GoLGrid *target_area, i32 glider_dir, u64 *projection, i32 projection_size)
{
	if (!target_area || GoLGrid_is_empty (target_area) || glider_dir < 0 || glider_dir >= 4 || !projection)
		return ffsc (__func__);
	
	if (glider_dir == 0 || glider_dir == 2)
		GoLGrid_make_rightup_projection (target_area, projection, projection_size);
	else
		GoLGrid_make_rightdown_projection (target_area, projection, projection_size);
	
	if (glider_dir == 0)
		return -9 - (2 * GoLGrid_get_rightdown_pop_off (target_area, projection, projection_size));
	else if (glider_dir == 1)
		return -11 + (2 * GoLGrid_get_rightup_pop_on (target_area, projection, projection_size));
	else if (glider_dir == 2)
		return -11 + (2 * GoLGrid_get_rightdown_pop_on (target_area, projection, projection_size));
	else
		return -9 - (2 * GoLGrid_get_rightup_pop_off (target_area, projection, projection_size));
}

// src_gg and dst_gg may mave different sizes. The virtual position of dst_gg is set to (0, 0) by this function. Returns FALSE if clipping occurred
static never_inline int GoLGrid_copy_to_top_left (const GoLGrid *src_gg, GoLGrid *dst_gg)
{
	if (!src_gg || !dst_gg)
		return ffsc (__func__);
	
	Rect src_bb;
	GoLGrid_get_bounding_box (src_gg, &src_bb);
	
	GoLGrid_set_grid_coords (dst_gg, 0, 0);
	return GoLGrid_copy_unmatched (src_gg, dst_gg, -src_bb.left_x, -src_bb.top_y);
}

// Only temp_gg_1 and temp_gg_2 need to be square and have the same grid rects. Returns FALSE if clipping occurred
static never_inline int GoLGrid_flip_diagonally_virtual (const GoLGrid *src_gg, GoLGrid *dst_gg, GoLGrid *temp_gg_1, GoLGrid *temp_gg_2)
{
	if (!src_gg || !dst_gg || !temp_gg_1 || temp_gg_1->grid_rect.height != temp_gg_1->grid_rect.width || !temp_gg_2 ||
			temp_gg_2->grid_rect.width != temp_gg_1->grid_rect.width || temp_gg_2->grid_rect.height != temp_gg_1->grid_rect.height)
		return ffsc (__func__);
	
	Rect src_grid_rect;
	GoLGrid_get_grid_rect (src_gg, &src_grid_rect);
	
	Rect src_bb;
	GoLGrid_get_bounding_box (src_gg, &src_bb);
	
	Rect temp_1_grid_rect;
	GoLGrid_get_grid_rect (temp_gg_1, &temp_1_grid_rect);
	
	int clipped = GoLGrid_copy_unmatched (src_gg, temp_gg_1, temp_1_grid_rect.left_x - src_bb.left_x, temp_1_grid_rect.top_y - src_bb.top_y);
	GoLGrid_flip_diagonally (temp_gg_1, temp_gg_2);
	clipped |= GoLGrid_copy_unmatched (temp_gg_2, dst_gg, src_bb.top_y - temp_1_grid_rect.left_x, src_bb.left_x - temp_1_grid_rect.top_y);
	
	return clipped;
}

// dst_grid, temp_gg_1 and temp_gg_2 must be square and have the same size, but may be of a different size than src_gg. The virtual position of dst_gg is set to (0, 0) by this function
// Returns the hash value of the canonical pattern
static never_inline u64 GoLGrid_make_canonical (const GoLGrid *src_gg, const RandomDataArray *rda, GoLGrid *temp_gg_1, GoLGrid *temp_gg_2, GoLGrid *dst_gg)
{
	if (!src_gg || !rda || !temp_gg_1 || temp_gg_1->grid_rect.height != temp_gg_1->grid_rect.width || !temp_gg_2 ||
			temp_gg_2->grid_rect.width != temp_gg_1->grid_rect.width || temp_gg_2->grid_rect.height != temp_gg_1->grid_rect.height ||
			(dst_gg && (dst_gg->grid_rect.width != temp_gg_1->grid_rect.width || dst_gg->grid_rect.height != temp_gg_1->grid_rect.height)))
		return ffsc (__func__);
	
	if (GoLGrid_is_empty (src_gg))
	{
		if (dst_gg)
			GoLGrid_copy_to_top_left (src_gg, dst_gg);
		
		// The hash of an empty grid is independent of all other aspects, so this is fine
		return GoLGrid_get_hash (src_gg, rda);
	}
	
	Rect bb;
	GoLGrid_copy_to_top_left (src_gg, temp_gg_1);
	GoLGrid_get_bounding_box (temp_gg_1, &bb);
	
	if (bb.height > bb.width)
	{
		GoLGrid_flip_diagonally (temp_gg_1, temp_gg_2);
		swap_pointers ((void **) &temp_gg_1, (void **) &temp_gg_2);
	}
	
	u64 lowest_hash = GoLGrid_int_try_canonical (temp_gg_1, NULL, rda, dst_gg);
	
	if (dst_gg)
		GoLGrid_copy (temp_gg_1, dst_gg);
	
	if (bb.height == bb.width)
	{
		GoLGrid_flip_vertically (temp_gg_1, temp_gg_2);
		GoLGrid_int_try_canonical (temp_gg_2, &lowest_hash, rda, dst_gg);
		GoLGrid_flip_horizontally (temp_gg_1, temp_gg_2);
		GoLGrid_int_try_canonical (temp_gg_2, &lowest_hash, rda, dst_gg);
		GoLGrid_flip_vertically (temp_gg_2, temp_gg_1);
		GoLGrid_int_try_canonical (temp_gg_1, &lowest_hash, rda, dst_gg);
		
		GoLGrid_flip_diagonally (temp_gg_2, temp_gg_1);
		GoLGrid_int_try_canonical (temp_gg_1, &lowest_hash, rda, dst_gg);
	}
	
	GoLGrid_flip_vertically (temp_gg_1, temp_gg_2);
	GoLGrid_int_try_canonical (temp_gg_2, &lowest_hash, rda, dst_gg);
	GoLGrid_flip_horizontally (temp_gg_1, temp_gg_2);
	GoLGrid_int_try_canonical (temp_gg_2, &lowest_hash, rda, dst_gg);
	GoLGrid_flip_vertically (temp_gg_2, temp_gg_1);
	GoLGrid_int_try_canonical (temp_gg_1, &lowest_hash, rda, dst_gg);
	
	return lowest_hash;
}

static never_inline int GoLGrid_or_filled_circle (GoLGrid *gg, double cent_x, double cent_y, double radius)
{
	if (!gg || !gg->grid || radius < 0.0)
		return ffsc (__func__);
	
	i32 y_on = round_double_to_i32 (cent_y - radius);
	i32 y_off = 1 + round_double_to_i32 (cent_y + radius);
	i32 x_on = round_double_to_i32 (cent_x - radius);
	i32 x_off = 1 + round_double_to_i32 (cent_x + radius);
	
	int not_clipped = TRUE;
	i32 y;
	i32 x;
	
	for (y = y_on; y < y_off; y++)
		for (x = x_on; x < x_off; x++)
			if (((double) x - cent_x) * ((double) x - cent_x) + ((double) y - cent_y) * ((double) y - cent_y) < (radius * radius))
				not_clipped &= GoLGrid_set_cell_on (gg, x, y);
	
	return not_clipped;
}

static never_inline void GoLGrid_print_full (const GoLGrid *gg, const Rect *print_rect, FILE *stream)
{
	if (!gg || !gg->grid || (print_rect != NULL && (print_rect->width < 0 || print_rect->height < 0)))
		return (void) ffsc (__func__);
	
	if (!stream)
		stream = stdout;
	
	if (GoLGrid_is_empty (gg))
	{
		fprintf (stream, "--- Empty grid\n\n");
		return;
	}
	
	Rect pr;
	if (print_rect)
		Rect_copy (print_rect, &pr);
	else
	{
		GoLGrid_get_bounding_box (gg, &pr);
		Rect_adjust_size (&pr, 2);
	}
	
	int y;
	int x;
	for (y = pr.top_y; y < pr.top_y + pr.height; y++)
	{
		for (x = pr.left_x; x < pr.left_x + pr.width; x++)
			fprintf (stream, "%c", (GoLGrid_get_cell (gg, x, y) != 0) ? '@' : '.');
		
		fprintf (stream, "\n");
	}
	
	fprintf (stream, "\n");
}

static never_inline void GoLGrid_print (const GoLGrid *gg)
{
	GoLGrid_print_full (gg, NULL, NULL);
}

static never_inline void GoLGrid_print_life_history_full (FILE *stream, const Rect *print_rect, const GoLGrid *on_gg, const GoLGrid *marked_gg, const GoLGrid *envelope_gg, const GoLGrid *special_gg,
		int include_header, int use_life_symbols, i32 max_line_length)
{
	if ((print_rect != NULL && (print_rect->width < 0 || print_rect->height < 0)) || (!on_gg && !marked_gg && !envelope_gg && !special_gg) ||
			(on_gg && !on_gg->grid) || (marked_gg && !marked_gg->grid) || (envelope_gg && !envelope_gg->grid) || (special_gg && !special_gg->grid))
		return (void) ffsc (__func__);
	
	if (!stream)
		stream = stdout;
	
	Rect pr;
	if (print_rect)
		Rect_copy (print_rect, &pr);
	else
	{
		Rect_make (&pr, 0, 0, 0, 0);
		GoLGrid_int_print_life_history_include_rect (&pr, on_gg);
		GoLGrid_int_print_life_history_include_rect (&pr, marked_gg);
		GoLGrid_int_print_life_history_include_rect (&pr, envelope_gg);
		GoLGrid_int_print_life_history_include_rect (&pr, special_gg);
	}
	
	if (include_header)
		fprintf (stream, "x = %d, y = %d, rule = LifeHistory\n", pr.width, pr.height);
	
	i32 line_length = 0;
	int unwritten_cell_state = 0;
	i32 unwritten_cell_count = 0;
	i32 unwritten_newline_count = 0;
	
	i32 y;
	i32 x;
	for (y = pr.top_y; y < pr.top_y + pr.height; y++)
	{
		for (x = pr.left_x; x < pr.left_x + pr.width; x++)
		{
			int cell_state = 0;
			
			if (on_gg && GoLGrid_get_cell (on_gg, x, y))
				cell_state = 1;
			
			if (marked_gg && GoLGrid_get_cell (marked_gg, x, y))
				cell_state = ((cell_state == 1) ? 3 : 4);
			
			if (special_gg && GoLGrid_get_cell (special_gg, x, y))
				cell_state = ((cell_state == 1 || cell_state == 3) ? 5 : 6);
			
			if (envelope_gg && cell_state == 0 && GoLGrid_get_cell (envelope_gg, x, y))
				cell_state = 2;
			
			if (unwritten_newline_count > 0 && cell_state != 0)
			{
				GoLGrid_int_print_life_history_symbol (stream, -1, use_life_symbols, unwritten_newline_count, max_line_length, &line_length);
				unwritten_newline_count = 0;
			}
			
			if (unwritten_cell_count > 0 && cell_state != unwritten_cell_state)
			{
				GoLGrid_int_print_life_history_symbol (stream, unwritten_cell_state, use_life_symbols, unwritten_cell_count, max_line_length, &line_length);
				unwritten_cell_count = 0;
			}
			
			unwritten_cell_state = cell_state;
			unwritten_cell_count++;
		}
		
		if (unwritten_cell_count > 0 && unwritten_cell_state != 0)
			GoLGrid_int_print_life_history_symbol (stream, unwritten_cell_state, use_life_symbols, unwritten_cell_count, max_line_length, &line_length);
		
		unwritten_cell_count = 0;
		unwritten_newline_count++;
	}
	
	fprintf (stream, "!\n");
}

static never_inline void GoLGrid_print_life_history (const GoLGrid *on_gg)
{
	GoLGrid_print_life_history_full (stdout, NULL, on_gg, NULL, NULL, NULL, TRUE, FALSE, 68);
}

// State 1: on_gg
// State 2: envelope_gg
// State 3: on_gg, marked_gg
// State 4: marked_gg
// State 5: on_gg, marked_gg, special_gg
// State 6: special_gg
static never_inline int GoLGrid_parse_life_history (const char *lh, i32 left_x, i32 top_y, GoLGrid *on_gg, GoLGrid *marked_gg, GoLGrid *envelope_gg, GoLGrid *special_gg, int *clipped, int *reinterpreted)
{
	if (clipped)
		*clipped = FALSE;
	
	if (reinterpreted)
		*reinterpreted = FALSE;
	
	if (!lh || (!on_gg && !marked_gg && !envelope_gg && !special_gg) || (on_gg && !on_gg->grid) || (marked_gg && !marked_gg->grid) || (envelope_gg && !envelope_gg->grid) || (special_gg && !special_gg->grid))
		return ffsc (__func__);
	
	if (on_gg)
		GoLGrid_clear (on_gg);
	if (marked_gg)
		GoLGrid_clear (marked_gg);
	if (envelope_gg)
		GoLGrid_clear (envelope_gg);
	if (special_gg)
		GoLGrid_clear (special_gg);
	
	int used_state [7];
	int state_ix;
	for (state_ix = 0; state_ix <= 6; state_ix++)
		used_state [state_ix] = FALSE;
	
	i32 cur_x = left_x;
	i32 cur_y = top_y;
	
	int overflow_x = FALSE;
	int overflow_y = FALSE;
	int not_clipped = TRUE;
	int success;
	
	while (TRUE)
	{
		int state;
		i32 count;
		
		if (!GoLGrid_int_get_life_history_symbol (&lh, &success, &state, &count))
			break;
		
		if (count == 0)
			continue;
		
		if (state == -1)
		{
			cur_x = left_x;
			overflow_x = FALSE;
			
			i64 new_y = (i64) cur_y + (i64) count;
			if (new_y > (i64) i32_MAX)
				overflow_y = TRUE;
			else
				cur_y = (i32) new_y;
		}
		else if (state == 0)
		{
			i64 new_x = (i64) cur_x + (i64) count;
			if (new_x > (i64) i32_MAX)
				overflow_x = TRUE;
			else
				cur_x = (i32) new_x;
		}
		else
		{
			i32 count_ix;
			for (count_ix = 0; count_ix < count; count_ix++)
			{
				if (overflow_y || overflow_x)
				{
					not_clipped = FALSE;
					
					// Note that this intentionally only breaks out from the for-loop, because integer overflow is not an error
					break;
				}
				else
				{
					used_state [state] = TRUE;
					
					if (on_gg && (state == 1 || state == 3 || state == 5))
						not_clipped &= GoLGrid_set_cell_on (on_gg, cur_x, cur_y);
					
					if (marked_gg && (state == 3 || state == 4 || state == 5))
						not_clipped &= GoLGrid_set_cell_on (marked_gg, cur_x, cur_y);
					
					if (envelope_gg && (state == 2))
						not_clipped &= GoLGrid_set_cell_on (envelope_gg, cur_x, cur_y);
					
					if (special_gg && (state == 5 || state == 6))
						not_clipped &= GoLGrid_set_cell_on (special_gg, cur_x, cur_y);
				}
				
				if (cur_x == i32_MAX)
					overflow_x = TRUE;
				else
					cur_x++;
			}
		}
	}
	
	if (!success)
	{
		if (clipped)
			*clipped = FALSE;
		
		if (on_gg)
			GoLGrid_clear (on_gg);
		if (marked_gg)
			GoLGrid_clear (marked_gg);
		if (envelope_gg)
			GoLGrid_clear (envelope_gg);
		if (special_gg)
			GoLGrid_clear (special_gg);
		
		return FALSE;
	}
	
	if (clipped)
		*clipped = !not_clipped;
	
	if (reinterpreted)
	{
		if (!on_gg && (used_state [1] || used_state [3] || used_state [5]))
			*reinterpreted = TRUE;
		if (!marked_gg && (used_state [3] || used_state [4] || (!special_gg && used_state [3] && used_state [5])))
			*reinterpreted = TRUE;
		if (!envelope_gg && used_state [2])
			*reinterpreted = TRUE;
		if (!special_gg && used_state [6])
			*reinterpreted = TRUE;
	}
	
	return TRUE;
}

static never_inline int GoLGrid_parse_life_history_simple (const char *lh, i32 left_x, i32 top_y, GoLGrid *on_gg)
{
	return GoLGrid_parse_life_history (lh, left_x, top_y, on_gg, NULL, NULL, NULL, NULL, NULL);
}
