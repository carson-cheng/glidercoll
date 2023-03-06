#include "std.c"
#include "parse.c"
#include "random.c"
#include "vectorize.c"
#include "file.c"
#include "randomarray.c"
#include "hashtable.c"
#include "rect.c"
#include "celllist.c"
#include "objects.c"
#include "golgrid.c"
#include "grid_nb_spec.c"
#include "gridmisc.c"
#include <time.h>
#define DATABASE_GRID_SIZE 2048
#define LINE_BUF_SIZE 256
#define SPEC_BUF_SIZE 131072
#define EVOLVE_GRID_HALFSIZE 160
#define MAX_PARTITIONS 16
#define MAX_ON_CELLS 40

#define MAX_GENS 384

#define CORNER_GRID_WIDTH 64
#define CORNER_GRID_HEIGHT 16

typedef struct
{
	RandomState random;
	RandomDataArray rda;
	HashTable_u64 corner_glider_ht;
	GoLGrid *corner_glider_mask [4];
	HashTable_void known_hashtable;
	HashTable_void new_hashtable;
	Rect field;
	Rect field_stop;
	GoLGrid *seed;
	i32 prev_cnt;
	i32 new_cnt;
} SearchState;

const char *corner_glider_spec [4] [4] = {{".A$2A$A.A!", "2A$A.A$A!", ".2A$2A$2.A!", "3A$A$.A!"},
										  {"62.A$62.2A$61.A.A!", "62.2A$61.A.A$63.A!", "61.2A$62.2A$61.A!", "61.3A$63.A$62.A!"},
										  {"13$61.A.A$62.2A$62.A!", "13$63.A$61.A.A$62.2A!", "13$61.A$62.2A$61.2A!", "13$62.A$63.A$61.3A!"},
										  {"13$A.A$2A$.A!", "13$A$A.A$2A!", "13$2.A$2A$.2A!", "13$.A$A$3A!"}};


static never_inline GoLGrid *alloc_database_grid (void)
{
	return GoLGrid_alloc_heap_obj (0, 0, DATABASE_GRID_SIZE, DATABASE_GRID_SIZE);
}

static never_inline GoLGrid *alloc_evolve_grid (void)
{
	return GoLGrid_alloc_heap_obj (-EVOLVE_GRID_HALFSIZE, -EVOLVE_GRID_HALFSIZE, 2 * EVOLVE_GRID_HALFSIZE, 2 * EVOLVE_GRID_HALFSIZE);
}

static never_inline GoLGrid *alloc_corner_grid (void)
{
	return GoLGrid_alloc_heap_obj (0, 0, CORNER_GRID_WIDTH, CORNER_GRID_HEIGHT);
}

static never_inline int is_connected (const GoLGrid *pattern)
{
	static int static_init = FALSE;
	static GoLGrid *all_bleed_3_or_more;
	static GoLGrid *cur_included;
	static GoLGrid *bleed_8;
	static GoLGrid *connecting_cells;
	static GoLGrid *new_included_mask;
	static GoLGrid *new_included;
	
	if (!static_init)
	{
		static_init = TRUE;
		all_bleed_3_or_more = alloc_evolve_grid ();
		cur_included = alloc_evolve_grid ();
		bleed_8 = alloc_evolve_grid ();
		connecting_cells = alloc_evolve_grid ();
		new_included_mask = alloc_evolve_grid ();
		new_included = alloc_evolve_grid ();
	}
	
	GoLGrid_bleed_3_or_more_neighbours (pattern, all_bleed_3_or_more);
	GoLGrid_clear (cur_included);
	
	i32 x = 0;
	i32 y = 0;
	GoLGrid_find_next_on_cell (pattern, TRUE, &x, &y);
	GoLGrid_set_cell_on (cur_included, x, y);
	
	while (TRUE)
	{
		GoLGrid_bleed_8 (cur_included, bleed_8);
		GoLGrid_and_dst (bleed_8, all_bleed_3_or_more, connecting_cells);
		GoLGrid_bleed_8 (connecting_cells, new_included_mask);
		GoLGrid_and_dst (pattern, new_included_mask, new_included);
		
		if (GoLGrid_is_equal (new_included, pattern))
			return TRUE;
		if (GoLGrid_is_equal (new_included, cur_included))
			return FALSE;
		
		GoLGrid_copy (new_included, cur_included);
	}
}

static never_inline i32 partition_into_islands (const GoLGrid *gg, ObjCellList *list)
{
	static int static_init = FALSE;
	static GoLGrid *remaining;
	static GoLGrid *cur_part;
	static GoLGrid *bleed_8;
	static GoLGrid *new_part;
	
	if (!static_init)
	{
		static_init = TRUE;
		remaining = alloc_evolve_grid ();
		cur_part = alloc_evolve_grid ();
		bleed_8 = alloc_evolve_grid ();
		new_part = alloc_evolve_grid ();
	}
	
	GoLGrid_copy (gg, remaining);
	
	i32 part_cnt = 0;
	while (TRUE)
	{
		if (GoLGrid_is_empty (remaining))
			break;
		
		i32 x;
		i32 y;
		if (!GoLGrid_find_next_on_cell (remaining, TRUE, &x, &y))
			break;
		
		GoLGrid_clear (cur_part);
		GoLGrid_set_cell_on (cur_part, x, y);
		
		while (TRUE)
		{
			GoLGrid_bleed_8 (cur_part, bleed_8);
			GoLGrid_and_dst (remaining, bleed_8, new_part);
			
			if (GoLGrid_is_equal (new_part, cur_part))
				break;
			
			GoLGrid_copy (new_part, cur_part);
		}
		
		GoLGrid_to_obj_cell_list (cur_part, &list [part_cnt]);
		GoLGrid_subtract_obj (remaining, cur_part);
		part_cnt++;
	}
	
	return part_cnt;
}

static never_inline int is_stable_subset (const ObjCellList *list, u64 partition_mask)
{
	static int static_init = FALSE;
	static GoLGrid *subset;
	static GoLGrid *evolved;
	
	if (!static_init)
	{
		static_init = TRUE;
		subset = alloc_evolve_grid ();
		evolved = alloc_evolve_grid ();
	}
	
	GoLGrid_clear (subset);
	while (partition_mask)
	{
		i32 list_ix = least_significant_bit_u64_fail_to_64 (partition_mask);
		GoLGrid_or_obj_cell_list (subset, &list [list_ix], 0, 0);
		partition_mask = partition_mask ^ (((u64) 1) << list_ix);
	}
	
	GoLGrid_evolve (subset, evolved);
	return GoLGrid_is_equal (subset, evolved);
}

static never_inline int has_stable_partitioning (const ObjCellList *list, i32 list_cnt, int allow_more_than_two_parts, int is_already_a_partition)
{
	if (is_already_a_partition)
	{
		if (is_stable_subset (list, (((u64) 1) << list_cnt) - (u64) 1))
			return TRUE;
		else if (!allow_more_than_two_parts)
			return FALSE;
	}
	
	i32 subset_cnt = 1 << list_cnt;
	i32 subset_ix;
	
	for (subset_ix = 1; subset_ix < subset_cnt - 2; subset_ix += 2)
		if (is_stable_subset (list, subset_ix))
		{
			ObjCellList obj [MAX_PARTITIONS];
			ObjCellList_Cell obj_cell [MAX_PARTITIONS] [MAX_ON_CELLS];
			
			i32 remains_cnt = 0;
			i32 remains_ix;
			for (remains_ix = 0; remains_ix < list_cnt; remains_ix++)
				if (((1 << remains_ix) & subset_ix) == 0)
				{
					ObjCellList_make_empty (&obj [remains_cnt], obj_cell [remains_cnt], MAX_ON_CELLS);
					ObjCellList_copy (&list [remains_ix], &obj [remains_cnt]);
					remains_cnt++;
				}
			
			if (has_stable_partitioning (obj, remains_cnt, allow_more_than_two_parts, TRUE))
				return TRUE;
		}
	
	return FALSE;	
}

static never_inline int is_pseudo_still (const GoLGrid *pattern)
{
	ObjCellList obj [MAX_PARTITIONS];
	ObjCellList_Cell obj_cell [MAX_PARTITIONS] [MAX_ON_CELLS];
	
	i32 obj_ix;
	for (obj_ix = 0; obj_ix < MAX_PARTITIONS; obj_ix++)
		ObjCellList_make_empty (&obj [obj_ix], obj_cell [obj_ix], MAX_ON_CELLS);
	
	i32 part_cnt = partition_into_islands (pattern, obj);
	return has_stable_partitioning (obj, part_cnt, TRUE, FALSE);
}

static never_inline int is_strict_still_life (const GoLGrid *pattern)
{
	if (!is_connected (pattern))
		return FALSE;
	
	return !is_pseudo_still (pattern);
}

static never_inline void cut_object (GoLGrid *pattern, i32 ref_x, i32 ref_y, GoLGrid *obj)
{
	static int static_init = FALSE;
	static GoLGrid *cur_cut;
	static GoLGrid *bleed_8;
	static GoLGrid *bleed_24;
	static GoLGrid *new_cut;
	
	if (!static_init)
	{
		static_init = TRUE;
		cur_cut = alloc_database_grid ();
		bleed_8 = alloc_database_grid ();
		bleed_24 = alloc_database_grid ();
		new_cut = alloc_database_grid ();
	}
	
	GoLGrid_clear (cur_cut);
	GoLGrid_set_cell_on (cur_cut, ref_x, ref_y);
	
	while (TRUE)
	{
		GoLGrid_bleed_8 (cur_cut, bleed_8);
		GoLGrid_bleed_8 (bleed_8, bleed_24);
		
		GoLGrid_and_dst (pattern, bleed_24, new_cut);
		if (GoLGrid_is_equal (new_cut, cur_cut))
			break;
		
		GoLGrid_copy (new_cut, cur_cut);
	}
	
	if (obj)
		GoLGrid_copy (new_cut, obj);
	
	GoLGrid_subtract_obj (pattern, new_cut);
}

static never_inline int enter_database_in_hashtable (SearchState *st, const char *database)
{
	GoLGrid *db = alloc_database_grid ();
	GoLGrid *db_evolved = alloc_database_grid ();
	GoLGrid *obj = alloc_database_grid ();
	GoLGrid *temp_1 = alloc_database_grid ();
	GoLGrid *temp_2 = alloc_database_grid ();
	
	if (!GoLGrid_parse_life_history_simple (database, 32, 32, db))
	{
		fprintf (stderr, "Failed to parse database\n");
		return FALSE;
	}
	
	i32 gen = 0;
	while (TRUE)
	{
		GoLGrid_evolve (db, db_evolved);
		gen++;
		
		if (GoLGrid_is_equal (db_evolved, db))
			break;
		
		if (gen >= MAX_GENS)
		{
			fprintf (stderr, "Database doesn't stabilize\n");
			return FALSE;
		}
		
		GoLGrid_copy (db_evolved, db);
	}
	
	i32 x = 0;
	i32 y = 0;
	i32 obj_cnt = 0;
	
	int first = TRUE;
	while (GoLGrid_find_next_on_cell (db, first, &x, &y))
	{
		first = FALSE;
		cut_object (db, x, y, obj);
		
		int was_present;
		HashTable_void_mark (&st->known_hashtable, GoLGrid_make_canonical (obj, &st->rda, temp_1, temp_2, NULL), &was_present);
		
		if (!was_present)
		{
			obj_cnt++;
			printf ("Database object %d, cell-count = %d:\n", obj_cnt, (i32) GoLGrid_get_population (obj));
			GoLGrid_print (obj);
		}
	}
	
	return TRUE;
}

static never_inline int will_collide (const Glider *gl_1, const Glider *gl_2)
{
	if (gl_1->dir == gl_2->dir)
		return FALSE;
	
	if ((gl_1->dir & 1) == (gl_2->dir & 1))
	{
		i32 gl_1_key = gl_1->lane + ((gl_1->dir == 0 || gl_1->dir == 3) ? 1 : 0);
		i32 gl_2_key = gl_2->lane + ((gl_2->dir == 0 || gl_2->dir == 3) ? 1 : 0);
		return (abs_i32_to_u32 (gl_1_key - gl_2_key) <= (u32) 6);
	}
	else if ((gl_1->dir & 2) == (gl_2->dir & 2))
	{
		i32 gl_1_key = gl_1->timing + (4 * gl_1->lane) * ((gl_1->dir == 0 || gl_1->dir == 3) ? -1 : 1);
		i32 gl_2_key = gl_2->timing + (4 * gl_2->lane) * ((gl_2->dir == 0 || gl_2->dir == 3) ? -1 : 1);
		return (abs_i32_to_u32 (gl_1_key - gl_2_key) <= (u32) 18);
	}
	else
		return (abs_i32_to_u32 (gl_1->timing - gl_2->timing) <= (u32) 18);
}

/*static never_inline int will_interfere (const Glider *gl_1, const Glider *gl_2)
{
	if (gl_1->dir != gl_2->dir)
		return FALSE;
	
	i32 delta_lane = (i32) abs_i32_to_u32 (gl_2->lane - gl_1->lane);
	if (delta_lane >= 7)
		return FALSE;
	
	i32 delta_progr = (i32) abs_i32_to_u32 (Objects_get_glider_progression (gl_2) - Objects_get_glider_progression (gl_1));
	return ((delta_progr <= 13 && delta_progr <= 18 - (2 * delta_lane)));
}*/

/*static never_inline void update_min_distance (const Glider *gl_1, const Glider *gl_2, i32 *min_distance)
{
	if (gl_1->dir == gl_2->dir)
		return;
	
	if (!will_collide (gl_1, gl_2))
		return;
	
	i32 gl_1_x;
	i32 gl_1_y;
	i32 gl_2_x;
	i32 gl_2_y;
	Objects_get_glider_center_coordinates (gl_1, &gl_1_x, &gl_1_y);
	Objects_get_glider_center_coordinates (gl_2, &gl_2_x, &gl_2_y);
	
	i32 dist = higher_of_i32 ((i32) abs_i32_to_u32 (gl_1_x - gl_2_x), (i32) abs_i32_to_u32 (gl_1_y - gl_2_y));
	if (*min_distance == -1 || dist < *min_distance)
		*min_distance = dist;
}*/

static never_inline int is_escaping_corner_glider (const SearchState *st, const GoLGrid *pattern, i32 dir, i32 extract_left_x, i32 extract_top_y)
{
	static int static_init = FALSE;
	static GoLGrid *extract;
	
	if (!static_init)
	{
		static_init = TRUE;
		extract = alloc_corner_grid ();
	}
	
	GoLGrid_copy_unmatched (pattern, extract, -extract_left_x, -extract_top_y);
	GoLGrid_subtract_obj (extract, st->corner_glider_mask [dir]);
	
	u64 glider_dir;
	if (HashTable_u64_get_data (&st->corner_glider_ht, GoLGrid_get_hash (extract, &st->rda), &glider_dir))
		if ((i32) glider_dir == dir)
			return TRUE;
	
	return FALSE;
}

static never_inline int has_escaping_corner_glider (const SearchState *st, const GoLGrid *pattern)
{
	Rect bb;
	GoLGrid_get_bounding_box (pattern, &bb);
	i32 x_on = bb.left_x;
	i32 x_off = bb.left_x + bb.width;
	i32 y_on = bb.top_y;
	i32 y_off = bb.top_y + bb.height;
	
	if (GoLGrid_get_cell (pattern, x_on + 1, y_on) && GoLGrid_get_cell (pattern, x_on, y_on + 1))
		if (is_escaping_corner_glider (st, pattern, 0, x_on, y_on))
			return TRUE;
	if (GoLGrid_get_cell (pattern, x_off - 2, y_on) && GoLGrid_get_cell (pattern, x_off - 1, y_on + 1))
		if (is_escaping_corner_glider (st, pattern, 1, x_off - CORNER_GRID_WIDTH, y_on))
			return TRUE;
	if (GoLGrid_get_cell (pattern, x_off - 2, y_off - 1) && GoLGrid_get_cell (pattern, x_off - 1, y_off - 2))
		if (is_escaping_corner_glider (st, pattern, 2, x_off - CORNER_GRID_WIDTH, y_off - CORNER_GRID_HEIGHT))
			return TRUE;
	if (GoLGrid_get_cell (pattern, x_on + 1, y_off - 1) && GoLGrid_get_cell (pattern, x_on, y_off - 2))
		if (is_escaping_corner_glider (st, pattern, 3, x_on, y_off - CORNER_GRID_HEIGHT))
			return TRUE;
	
	return FALSE;
}
int main (int argc, const char *const *argv)
{
	
}
