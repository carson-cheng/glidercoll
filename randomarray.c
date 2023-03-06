// A datatype for a pre-generated array of random u64 data
// The typical way to make use of this is to verify just once that the size is enough with RandomDataArray_verify_size, then access the array directly with rda_object->random_data [index]
// By specifying a max_grid_height_for_compatibility greater than height_in_bits, it is possible to ensure that hashes are identical when generated from identical grid patterns on grids of
// different height up to max_grid_height_for_compatibility

typedef struct
{
	u64 *data_alloc;
	u64 *data;
	i64 col_cnt;
	i64 height;
	i64 max_grid_height_for_compatibility;
} RandomDataArray;

static never_inline void RandomDataArray_preinit (RandomDataArray *rda)
{
	if (!rda)
		return (void) ffsc (__func__);
	
	rda->data_alloc = NULL;
	rda->data = NULL;
	rda->col_cnt = 0;
	rda->height = 0;
	rda->max_grid_height_for_compatibility = 0;
}

static never_inline void RandomDataArray_free (RandomDataArray *rda)
{
	if (!rda)
		return (void) ffsc (__func__);
	
	if (rda->data_alloc)
		free (rda->data_alloc);
	
	RandomDataArray_preinit (rda);
}

static never_inline int RandomDataArray_create_int (RandomDataArray *rda, RandomState *rs, i64 col_cnt, i64 height, i64 max_grid_height_for_compatibility)
{
	RandomDataArray_preinit (rda);
	
	rda->col_cnt = col_cnt;
	rda->height = height;
	rda->max_grid_height_for_compatibility = max_grid_height_for_compatibility;
	
	if (!allocate_aligned ((col_cnt * height) * sizeof (u64), TYPICAL_CACHE_LINE_BYTE_SIZE, 0, FALSE, (void **) &rda->data_alloc, (void **) &rda->data))
	{
		fprintf (stderr, "Out of memory in %s\n", __func__);
		RandomDataArray_free (rda);
		return FALSE;
	}
	
	i64 col_ix;
	i64 word_ix;
	for (col_ix = 0; col_ix < col_cnt; col_ix++)
		for (word_ix = 0; word_ix < max_grid_height_for_compatibility; word_ix++)
		{
			u64 word = Random_get (rs);
			if (word_ix < height)
				rda->data [(height * col_ix) + word_ix] = word;
		}
	
	return TRUE;
}

static never_inline int RandomDataArray_create_for_u64_grid (RandomDataArray *rda, i32 width_in_bits, i32 height_in_bits, i32 max_height_for_compatibility, RandomState *random)
{
	if (!rda || width_in_bits <= 0 || width_in_bits % 64 != 0 || height_in_bits <= 0 || (max_height_for_compatibility != -1 && max_height_for_compatibility < height_in_bits) || !random)
		return ffsc (__func__);
	
	if (max_height_for_compatibility == -1)
		max_height_for_compatibility = height_in_bits;
	
	return RandomDataArray_create_int (rda, random, (i64) (width_in_bits / 64), (i64) height_in_bits, (i64) max_height_for_compatibility);
}

static always_inline int RandomDataArray_verify_for_u64_grid (const RandomDataArray *rda, i32 width_in_bits, i32 height_in_bits)
{
	if (!rda || !rda->data)
		return ffsc (__func__);
	
	return (rda->col_cnt >= (i64) ((width_in_bits + 63) / 64) && rda->height >= (i64) height_in_bits);
}
