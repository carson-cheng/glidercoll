// A description of a glider
// (dir) is 0 for a NW, 1 for a NE, 2 for a SE and 3 for SW-bound glider
// (lane) is the x-coordinate for the center cell of the glider if it is moved backwards or forwards in time, so that its center cell has y-coordinate 0
// and it is in the phase with three cells in a horizontal line
// (timing) is the generation if the glider is moved backwards or forwards in time, so that its center cell has x-coordinate 0 (instead of y-coordinate
// as for (lane)) and with the same phase as above

// 2017-07-31: Preparing for comes_close and interferes
//	static never_inline int will_come_close (const Glider *gl_1, const Glider *gl_2)
//	{
//		// Tested with random input and works, returns TRUE if at least one off-cell will be in the neighbourhood of both gliders at some point
//		
//		if (gl_1->dir == gl_2->dir)
//			return ffsc (__func__);
//		
//		if ((gl_1->dir & 1) == (gl_2->dir & 1))
//		{
//			i32 gl_1_key = gl_1->lane + ((gl_1->dir == 0 || gl_1->dir == 3) ? 1 : 0);
//			i32 gl_2_key = gl_2->lane + ((gl_2->dir == 0 || gl_2->dir == 3) ? 1 : 0);
//			return (abs_i32 (gl_1_key - gl_2_key) <= 7);
//		}
//		else if ((gl_1->dir & 2) == (gl_2->dir & 2))
//		{
//			i32 gl_1_key = gl_1->timing + (4 * gl_1->lane) * ((gl_1->dir == 0 || gl_1->dir == 3) ? -1 : 1);
//			i32 gl_2_key = gl_2->timing + (4 * gl_2->lane) * ((gl_2->dir == 0 || gl_2->dir == 3) ? -1 : 1);
//			return (abs_i32 (gl_1_key - gl_2_key) <= 19);
//		}
//		else
//			return (abs_i32 (gl_1->timing - gl_2->timing) <= 19);
//	}

typedef struct
{
	i32 dir;
	i32 lane;
	i32 timing;
} Glider;

typedef struct
{
	const ObjCellList_Cell cells [5];
	i32 x_offs;
	i32 y_offs;
	i32 lane_y_dir;
	i32 timing_x_dir;
	i32 timing_y_dir;
} Objects_GliderData;

static const Objects_GliderData Objects_glider_data [4] [4] =
{{{{{0, 0}, {1, 0}, {2, 0}, {0, 1}, {1, 2}}, -1, -1, -1,  1,  1}, {{{1, 0}, {2, 0}, {0, 1}, {1, 1}, {2, 2}}, -1, -1, -1,  1,  1},
  {{{0, 0}, {1, 0}, {0, 1}, {2, 1}, {0, 2}},  0, -1, -1,  1,  1}, {{{1, 0}, {0, 1}, {1, 1}, {0, 2}, {2, 2}},  0, -1, -1,  1,  1}},
 {{{{0, 0}, {1, 0}, {2, 0}, {2, 1}, {1, 2}}, -1, -1,  1, -1,  1}, {{{0, 0}, {1, 0}, {1, 1}, {2, 1}, {0, 2}}, -1, -1,  1, -1,  1},
  {{{1, 0}, {2, 0}, {0, 1}, {2, 1}, {2, 2}}, -2, -1,  1, -1,  1}, {{{1, 0}, {1, 1}, {2, 1}, {0, 2}, {2, 2}}, -2, -1,  1, -1,  1}},
 {{{{1, 0}, {2, 1}, {0, 2}, {1, 2}, {2, 2}}, -1, -1, -1, -1, -1}, {{{0, 0}, {1, 1}, {2, 1}, {0, 2}, {1, 2}}, -1, -1, -1, -1, -1},
  {{{2, 0}, {0, 1}, {2, 1}, {1, 2}, {2, 2}}, -2, -1, -1, -1, -1}, {{{0, 0}, {2, 0}, {1, 1}, {2, 1}, {1, 2}}, -2, -1, -1, -1, -1}},
 {{{{1, 0}, {0, 1}, {0, 2}, {1, 2}, {2, 2}}, -1, -1,  1,  1, -1}, {{{2, 0}, {0, 1}, {1, 1}, {1, 2}, {2, 2}}, -1, -1,  1,  1, -1},
  {{{0, 0}, {0, 1}, {2, 1}, {0, 2}, {1, 2}},  0, -1,  1,  1, -1}, {{{0, 0}, {2, 0}, {0, 1}, {1, 1}, {1, 2}},  0, -1,  1,  1, -1}}};


static never_inline void Objects_get_glider_timing_range (i32 glider_dir, const Rect *allowed_rect, i32 *timing_on, i32 *timing_off)
{
	if (timing_on)
		*timing_on = 0;
	if (timing_off)
		*timing_off = 0;
	
	if (glider_dir < 0 || glider_dir >= 4 || !allowed_rect || !timing_on || !timing_off)
		return (void) ffsc (__func__);
	
	if (glider_dir == 0 || glider_dir == 3)
	{
		*timing_on = (4 * allowed_rect->left_x) - 6;
		*timing_off = (4 * (allowed_rect->left_x + allowed_rect->width)) + 2;
	}
	else
	{
		*timing_on = -(4 * (allowed_rect->left_x + allowed_rect->width)) - 2;
		*timing_off = -(4 * allowed_rect->left_x) + 6;
	}
}

static always_inline i32 Objects_get_glider_progression (const Glider *gl)
{
	if (!gl || gl->dir < 0 || gl->dir >= 4)
		return ffsc (__func__);
	
	return (-gl->timing + (Objects_glider_data [gl->dir] [0].timing_x_dir * (2 * gl->lane)));
}

static always_inline void Objects_set_glider_progression (Glider *gl, i32 progression)
{
	if (!gl || gl->dir < 0 || gl->dir >= 4)
		return (void) ffsc (__func__);
	
	gl->timing = -progression + (Objects_glider_data [gl->dir] [0].timing_x_dir * (2 * gl->lane));
}

static always_inline void Objects_shift_glider (Glider *gl, i32 offs_x, i32 offs_y)
{
	if (!gl || gl->dir < 0 || gl->dir >= 4)
		return (void) ffsc (__func__);
	
	gl->lane += (offs_x + (Objects_glider_data [gl->dir] [0].lane_y_dir * offs_y));
	gl->timing += 4 * (Objects_glider_data [gl->dir] [0].timing_x_dir * offs_x);
}

static always_inline void Objects_mirror_glider (Glider *gl)
{
	if (!gl || gl->dir < 0 || gl->dir >= 4)
		return (void) ffsc (__func__);
	
	i32 lane_offs = (-gl->lane - 1) - gl->lane;
	gl->lane += lane_offs;
	gl->timing += 2 * (Objects_glider_data [gl->dir] [0].timing_x_dir * lane_offs);
}

static always_inline void Objects_get_glider_center_coordinates (const Glider *gl, i32 *center_x, i32 *center_y)
{
	if (center_x)
		*center_x = 0;
	if (center_y)
		*center_y = 0;
	
	if (!gl || gl->dir < 0 || gl->dir >= 4 || !center_x || !center_y)
		return (void) ffsc (__func__);
	
	i32 timing_phase = gl->timing & 3;
	i32 timing_step = (gl->timing - timing_phase) / 4;
	
	const Objects_GliderData *gl_data = &Objects_glider_data [gl->dir] [timing_phase];
	
	*center_x = 1 + (gl_data->x_offs + (gl_data->timing_x_dir * timing_step));
	*center_y = 1 + (gl_data->y_offs +  (gl_data->lane_y_dir * gl->lane) + (gl_data->timing_y_dir * timing_step));
}

// The cell list is fully initialized by this call and ocl->cell is set to point to a static const object, so the cell list may not be modified
static always_inline void Objects_make_glider_obj_cell_list (ObjCellList *ocl, const Glider *gl)
{
	if (!ocl || !gl || gl->dir < 0 || gl->dir >= 4)
	{
		if (ocl)
			ObjCellList_make_zero_size (ocl);
		
		return (void) ffsc (__func__);
	}
	
	i32 timing_phase = gl->timing & 3;
	i32 timing_step = (gl->timing - timing_phase) / 4;
	
	const Objects_GliderData *gl_data = &Objects_glider_data [gl->dir] [timing_phase];
	
	i32 left_x = gl_data->x_offs + (gl_data->timing_x_dir * timing_step);
	i32 top_y = gl_data->y_offs +  (gl_data->lane_y_dir * gl->lane) + (gl_data->timing_y_dir * timing_step);
	
	Rect_make (&ocl->obj_rect, left_x, top_y, 3, 3);
	ocl->cell_cnt = 5;
	ocl->max_cells = 5;
	
	// To initialize a standard ObjCellList with static const cells we must cast away the constness
	ocl->cell = (ObjCellList_Cell *) gl_data->cells;
}
