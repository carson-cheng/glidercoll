typedef struct
{
	u64 key;
} HashTable_void_Entry;

typedef struct
{
	double reallocate_filled_part;
	double fail_filled_part;
	u64 total_capacity;
	u64 reallocate_capacity;
	u64 fail_capacity;
	u64 used_capacity;
	int failed_reallocation;
	HashTable_void_Entry *table;
} HashTable_void;

typedef struct
{
	u64 key;
	u64 data;
} HashTable_u64_Entry;

typedef struct
{
	double reallocate_filled_part;
	double fail_filled_part;
	u64 total_capacity;
	u64 reallocate_capacity;
	u64 fail_capacity;
	u64 used_capacity;
	int failed_reallocation;
	HashTable_u64_Entry *table;
} HashTable_u64;

// FIXME: Change externally visible sizes from u64 to s64
// FIXME: Make sure fail_capacity < 2 * used_capacity so that a reallocation can't cause an internal reallocation


static never_inline void HashTable_void_preinit (HashTable_void *ht)
{
	if (!ht)
		return (void) ffsc (__func__);
	
	ht->reallocate_filled_part = 0.0;
	ht->fail_filled_part = 0.0;
	ht->total_capacity = 0;
	ht->reallocate_capacity = 0;
	ht->fail_capacity = 0;
	ht->used_capacity = 0;
	ht->failed_reallocation = FALSE;
	ht->table = NULL;
}

static never_inline void HashTable_void_free (HashTable_void *ht)
{
	if (!ht)
		return (void) ffsc (__func__);
	
	if (ht->table)
		free (ht->table);
	
	HashTable_void_preinit (ht);
}

static never_inline void HashTable_void_clear (HashTable_void *ht)
{
	if (!ht || !ht->table)
		return (void) ffsc (__func__);
	
	ht->used_capacity = 0;
	ht->failed_reallocation = FALSE;
	
	u64 entry_ix;
	for (entry_ix = 0; entry_ix < ht->total_capacity; entry_ix++)
		ht->table [entry_ix].key = 0;
}

static never_inline int HashTable_void_allocate (HashTable_void *ht, u64 capacity)
{
	if (!ht || ht->table != NULL || bit_count_u64 (capacity) != 1)
		return ffsc (__func__);
	
	ht->table = malloc (capacity * sizeof (HashTable_void_Entry));
	if (!ht->table)
		return FALSE;
	
	ht->total_capacity = capacity;
	ht->reallocate_capacity = lower_of_u64 (capacity - 1, (u64) (ht->reallocate_filled_part * (double) capacity));
	ht->fail_capacity = lower_of_u64 (capacity - 1, (u64) (ht->fail_filled_part * (double) capacity));
	
	HashTable_void_clear (ht);
	
	return TRUE;
}

static never_inline int HashTable_void_create (HashTable_void *ht, u64 first_capacity, double reallocate_filled_part, double fail_filled_part)
{
	if (!ht)
		return ffsc (__func__);
	
	HashTable_void_preinit (ht);
	
	if (reallocate_filled_part < 0.25 || reallocate_filled_part > 1.0 || fail_filled_part < 0.25 || fail_filled_part > 1.0 || fail_filled_part < reallocate_filled_part)
		return ffsc (__func__);
	
	ht->reallocate_filled_part = reallocate_filled_part;
	ht->fail_filled_part = fail_filled_part;
	
	if (!HashTable_void_allocate (ht, first_capacity))
	{
		fprintf (stderr, "Out of memory in %s\n", __func__);
		HashTable_void_free (ht);
		return FALSE;
	}
	
	return TRUE;
}

static always_inline int HashTable_void_mark (HashTable_void *ht, u64 key, int *was_marked);
static never_inline int HashTable_void_reallocate (HashTable_void *ht, u64 new_capacity)
{
	if (!ht || !ht->table || new_capacity <= ht->total_capacity)
		return ffsc (__func__);
	
	HashTable_void temp_ht;
	HashTable_void_preinit (&temp_ht);
	temp_ht.reallocate_filled_part = ht->reallocate_filled_part;
	temp_ht.fail_filled_part = ht->fail_filled_part;
	
	ht->failed_reallocation = !(HashTable_void_allocate (&temp_ht, new_capacity));
	if (ht->failed_reallocation)
		return FALSE;
	
	u64 entry_ix;
	for (entry_ix = 0; entry_ix < ht->total_capacity; entry_ix++)
	{
		u64 entry_key = ht->table [entry_ix].key;
		if (entry_key != 0)
			HashTable_void_mark (&temp_ht, entry_key, NULL);
	}
	
	ht->total_capacity = temp_ht.total_capacity;
	ht->reallocate_capacity = temp_ht.reallocate_capacity;
	ht->fail_capacity = temp_ht.fail_capacity;
	ht->used_capacity = temp_ht.used_capacity;
	
	free (ht->table);
	ht->table = temp_ht.table;
	
	return TRUE;
}

static always_inline int HashTable_void_is_marked (const HashTable_void *ht, u64 key)
{
	if (!ht || !ht->table || key == 0)
		return ffsc (__func__);
	
	u64 entry_ix = key & (ht->total_capacity - 1);
	while (TRUE)
	{
		if (ht->table [entry_ix].key == key)
			return TRUE;
		else if (ht->table [entry_ix].key == 0)
			return FALSE;
		
		entry_ix = (entry_ix + 1) & (ht->total_capacity - 1);
	}
}

static never_inline u64 HashTable_void_memory_size (const HashTable_void *ht)
{
	if (!ht)
		return ffsc (__func__);
	
	return ht->total_capacity * sizeof (HashTable_void_Entry);
}

static never_inline double HashTable_void_used_ratio (const HashTable_void *ht)
{
	if (!ht)
		return ffsc (__func__);
	
	return ((double) ht->used_capacity) / (double) ht->total_capacity;
}

static always_inline int HashTable_void_mark (HashTable_void *ht, u64 key, int *was_marked)
{
	if (was_marked)
		*was_marked = FALSE;
	
	if (!ht || !ht->table || key == 0)
		return ffsc (__func__);
	
	if (ht->used_capacity >= ht->reallocate_capacity && !ht->failed_reallocation)
		HashTable_void_reallocate (ht, 2 * ht->total_capacity);
	
	if (ht->used_capacity >= ht->fail_capacity)
		if (!HashTable_void_reallocate (ht, 2 * ht->total_capacity))
		{
			fprintf (stderr, "Out of memory in %s\n", __func__);
			return FALSE;
		}
	
	u64 entry_ix = key & (ht->total_capacity - 1);
	while (TRUE)
	{
		if (ht->table [entry_ix].key == key)
		{
			if (was_marked)
				*was_marked = TRUE;
			
			return TRUE;
		}
		else if (ht->table [entry_ix].key == 0)
		{
			ht->table [entry_ix].key = key;
			
			ht->used_capacity++;
			return TRUE;
		}
		
		entry_ix = (entry_ix + 1) & (ht->total_capacity - 1);
	}
}

static never_inline int HashTable_void_iterate (const HashTable_void *ht, u64 *entry_ix, u64 *key)
{
	if (!ht || !ht->table || !entry_ix || *entry_ix >= ht->total_capacity || !key)
		return ffsc (__func__);
	
	while (TRUE)
	{
		u64 entry_key = ht->table [*entry_ix].key;
		if (entry_key != *key && entry_key != 0)
		{
			*key = entry_key;
			return TRUE;
		}
		
		(*entry_ix)++;
		if (*entry_ix >= ht->total_capacity)
			return FALSE;
	}
}

static never_inline void HashTable_u64_preinit (HashTable_u64 *ht)
{
	if (!ht)
		return (void) ffsc (__func__);
	
	ht->reallocate_filled_part = 0.0;
	ht->fail_filled_part = 0.0;
	ht->total_capacity = 0;
	ht->reallocate_capacity = 0;
	ht->fail_capacity = 0;
	ht->used_capacity = 0;
	ht->failed_reallocation = FALSE;
	ht->table = NULL;
}

static never_inline void HashTable_u64_free (HashTable_u64 *ht)
{
	if (!ht)
		return (void) ffsc (__func__);
	
	if (ht->table)
		free (ht->table);
	
	HashTable_u64_preinit (ht);
}

static never_inline void HashTable_u64_clear (HashTable_u64 *ht)
{
	if (!ht || !ht->table)
		return (void) ffsc (__func__);
	
	ht->used_capacity = 0;
	ht->failed_reallocation = FALSE;
	
	u64 entry_ix;
	for (entry_ix = 0; entry_ix < ht->total_capacity; entry_ix++)
	{
		ht->table [entry_ix].key = 0;
		ht->table [entry_ix].data = 0;
	}
}

static never_inline int HashTable_u64_allocate (HashTable_u64 *ht, u64 capacity)
{
	if (!ht || ht->table != NULL || bit_count_u64 (capacity) != 1)
		return ffsc (__func__);
	
	ht->table = malloc (capacity * sizeof (HashTable_u64_Entry));
	if (!ht->table)
		return FALSE;
	
	ht->total_capacity = capacity;
	ht->reallocate_capacity = lower_of_u64 (capacity - 1, (u64) (ht->reallocate_filled_part * (double) capacity));
	ht->fail_capacity = lower_of_u64 (capacity - 1, (u64) (ht->fail_filled_part * (double) capacity));
	
	HashTable_u64_clear (ht);
	
	return TRUE;
}

static never_inline int HashTable_u64_create (HashTable_u64 *ht, u64 first_capacity, double reallocate_filled_part, double fail_filled_part)
{
	if (!ht)
		return ffsc (__func__);
	
	HashTable_u64_preinit (ht);
	
	if (reallocate_filled_part < 0.25 || reallocate_filled_part > 1.0 || fail_filled_part < 0.25 || fail_filled_part > 1.0 || fail_filled_part < reallocate_filled_part)
		return ffsc (__func__);
	
	ht->reallocate_filled_part = reallocate_filled_part;
	ht->fail_filled_part = fail_filled_part;
	
	if (!HashTable_u64_allocate (ht, first_capacity))
	{
		fprintf (stderr, "Out of memory in %s\n", __func__);
		HashTable_u64_free (ht);
		return FALSE;
	}
	
	return TRUE;
}

static always_inline int HashTable_u64_store (HashTable_u64 *ht, u64 key, u64 data, int replace_previous_data, int *was_present);
static never_inline int HashTable_u64_reallocate (HashTable_u64 *ht, u64 new_capacity)
{
	if (!ht || !ht->table || new_capacity <= ht->total_capacity)
		return ffsc (__func__);
	
	HashTable_u64 temp_ht;
	HashTable_u64_preinit (&temp_ht);
	temp_ht.reallocate_filled_part = ht->reallocate_filled_part;
	temp_ht.fail_filled_part = ht->fail_filled_part;
	
	ht->failed_reallocation = !(HashTable_u64_allocate (&temp_ht, new_capacity));
	if (ht->failed_reallocation)
		return FALSE;
	
	u64 entry_ix;
	for (entry_ix = 0; entry_ix < ht->total_capacity; entry_ix++)
	{
		u64 entry_key = ht->table [entry_ix].key;
		if (entry_key != 0)
			HashTable_u64_store (&temp_ht, entry_key, ht->table [entry_ix].data, TRUE, NULL);
	}
	
	ht->total_capacity = temp_ht.total_capacity;
	ht->reallocate_capacity = temp_ht.reallocate_capacity;
	ht->fail_capacity = temp_ht.fail_capacity;
	ht->used_capacity = temp_ht.used_capacity;
	
	free (ht->table);
	ht->table = temp_ht.table;
	
	return TRUE;
}

static always_inline int HashTable_u64_get_data (const HashTable_u64 *ht, u64 key, u64 *data)
{
	if (data)
		*data = 0;
	
	if (!ht || !ht->table || key == 0)
		return ffsc (__func__);
	
	u64 entry_ix = key & (ht->total_capacity - 1);
	while (TRUE)
	{
		if (ht->table [entry_ix].key == key)
		{
			if (data)
				*data = ht->table [entry_ix].data;
			
			return TRUE;
		}
		else if (ht->table [entry_ix].key == 0)
			return FALSE;
		
		entry_ix = (entry_ix + 1) & (ht->total_capacity - 1);
	}
}

static never_inline u64 HashTable_u64_memory_size (const HashTable_u64 *ht)
{
	if (!ht)
		return ffsc (__func__);
	
	return ht->total_capacity * sizeof (HashTable_u64_Entry);
}

static never_inline double HashTable_u64_used_ratio (const HashTable_u64 *ht)
{
	if (!ht)
		return ffsc (__func__);
	
	return ((double) ht->used_capacity) / (double) ht->total_capacity;
}

static always_inline int HashTable_u64_store (HashTable_u64 *ht, u64 key, u64 data, int replace_previous_data, int *was_present)
{
	if (was_present)
		*was_present = FALSE;
	
	if (!ht || !ht->table || key == 0)
		return ffsc (__func__);
	
	if (ht->used_capacity >= ht->reallocate_capacity && !ht->failed_reallocation)
		HashTable_u64_reallocate (ht, 2 * ht->total_capacity);
	
	if (ht->used_capacity >= ht->fail_capacity)
		if (!HashTable_u64_reallocate (ht, 2 * ht->total_capacity))
		{
			fprintf (stderr, "Out of memory in %s\n", __func__);
			return FALSE;
		}
	
	u64 entry_ix = key & (ht->total_capacity - 1);
	while (TRUE)
	{
		if (ht->table [entry_ix].key == key)
		{
			if (replace_previous_data)
				ht->table [entry_ix].data = data;
			
			if (was_present)
				*was_present = TRUE;
			
			return TRUE;
		}
		else if (ht->table [entry_ix].key == 0)
		{
			ht->table [entry_ix].key = key;
			ht->table [entry_ix].data = data;
			
			ht->used_capacity++;
			return TRUE;
		}
		
		entry_ix = (entry_ix + 1) & (ht->total_capacity - 1);
	}
}

static never_inline int HashTable_u64_iterate (const HashTable_u64 *ht, u64 *entry_ix, u64 *key, u64 *data)
{
	if (data)
		*data = 0;
	
	if (!ht || !ht->table || !entry_ix || *entry_ix >= ht->total_capacity || !key)
		return ffsc (__func__);
	
	while (TRUE)
	{
		u64 entry_key = ht->table [*entry_ix].key;
		if (entry_key != *key && entry_key != 0)
		{
			*key = entry_key;
			if (data)
				*data = ht->table [*entry_ix].data;
			
			return TRUE;
		}
		
		(*entry_ix)++;
		if (*entry_ix >= ht->total_capacity)
			return FALSE;
	}
}

// FIXME: Use a more direct approach to close the gap from the removed entry
static never_inline int HashTable_u64_remove (HashTable_u64 *ht, u64 key)
{
	if (!ht || !ht->table || key == 0)
		return ffsc (__func__);
	
	u64 entry_ix = key & (ht->total_capacity - 1);
	while (TRUE)
	{
		if (ht->table [entry_ix].key == key)
			break;
		else if (ht->table [entry_ix].key == 0)
			return FALSE;
		
		entry_ix = (entry_ix + 1) & (ht->total_capacity - 1);
	}
	
	ht->table [entry_ix].key = 0;
	ht->table [entry_ix].data = 0;
	ht->used_capacity--;
	
	while (TRUE)
	{
		entry_ix = (entry_ix + 1) & (ht->total_capacity - 1);
		if (ht->table [entry_ix].key == 0)
			break;
		
		u64 reenter_key = ht->table [entry_ix].key;
		u64 reenter_data = ht->table [entry_ix].data;
		
		ht->table [entry_ix].key = 0;
		ht->table [entry_ix].data = 0;
		ht->used_capacity--;
		
		HashTable_u64_store (ht, reenter_key, reenter_data, TRUE, NULL);
	}
	
	return TRUE;
}
