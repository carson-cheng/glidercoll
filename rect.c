// Note that Rects with no area are supported and still retain a position - for example two 0-by-0 Rects can be different

typedef struct
{
	i32 left_x;
	i32 top_y;
	i32 width;
	i32 height;
} Rect;

static always_inline void Rect_make (Rect *r, i32 left_x, i32 top_y, i32 width, i32 height)
{
	if (!r)
		return (void) ffsc (__func__);
	
	if (width < 0 || height < 0)
	{
		width = 0;
		height = 0;
		ffsc (__func__);
	}
	
	r->left_x = left_x;
	r->top_y = top_y;
	r->width = width;
	r->height = height;
}

static always_inline int Rect_is_empty (const Rect *r)
{
	if (!r)
		return ffsc (__func__);
	
	return (r->width == 0);
}

static always_inline int Rect_is_equal (const Rect *rect_1, const Rect *rect_2)
{
	if (!rect_1 || !rect_2)
		return ffsc (__func__);
	
	return (rect_1->left_x == rect_2->left_x && rect_1->top_y == rect_2->top_y && rect_1->width == rect_2->width && rect_1->height == rect_2->height);
}

static always_inline int Rect_within (const Rect *r, i32 x, i32 y)
{
	return (x >= r->left_x && x < (r->left_x + r->width) && y >= r->top_y && y < (r->top_y + r->height));
}

static always_inline int Rect_is_subset (const Rect *obj, const Rect *ref)
{
	if (!obj || !ref)
		return ffsc (__func__);
	
	return (obj->left_x >= ref->left_x && obj->left_x + obj->width <= ref->left_x + ref->width && obj->top_y >= ref->top_y && obj->top_y + obj->height <= ref->top_y + ref->height);
}

static always_inline void Rect_copy (const Rect *src, Rect *dst)
{
	if (!src || !dst)
		return (void) ffsc (__func__);
	
	dst->left_x = src->left_x;
	dst->top_y = src->top_y;
	dst->width = src->width;
	dst->height = src->height;
}

// dst is allowed to alias src. If src have zero area, its position will be ignored and dst will be a 1-by-1 Rect
static always_inline void Rect_include (const Rect *src, Rect *dst, i32 x, i32 y)
{
	if (!src || !dst)
		return (void) ffsc (__func__);
	
	if (src->width <= 0 || src->height <= 0)
	{
		dst->left_x = x;
		dst->top_y = y;
		dst->width = 1;
		dst->height = 1;
	}
	else
	{
		i32 left_x = lower_of_i32 (src->left_x, x);
		i32 x_off = higher_of_i32 (src->left_x + src->width, x + 1);
		i32 top_y = lower_of_i32 (src->top_y, y);
		i32 y_off = higher_of_i32 (src->top_y + src->height, y + 1);
		
		dst->left_x = left_x;
		dst->top_y = top_y;
		dst->width = x_off - left_x;
		dst->height = y_off - top_y;
	}
}

// dst is allowed to alias either src
static always_inline void Rect_union (const Rect *src_1, const Rect *src_2, Rect *dst)
{
	if (!src_1 || !src_2 || !dst)
		return (void) ffsc (__func__);
	
	i32 left_x = lower_of_i32 (src_1->left_x, src_2->left_x);
	i32 x_off = higher_of_i32 (src_1->left_x + src_1->width, src_2->left_x + src_2->width);
	i32 top_y = lower_of_i32 (src_1->top_y, src_2->top_y);
	i32 y_off = higher_of_i32 (src_1->top_y + src_1->height, src_2->top_y + src_2->height);
	
	dst->left_x = left_x;
	dst->top_y = top_y;
	dst->width = x_off - left_x;
	dst->height = y_off - top_y;
}

// dst is allowed to alias either src. Returns TRUE of the result is non-empty
static always_inline int Rect_intersection (const Rect *src_1, const Rect *src_2, Rect *dst)
{
	if (!src_1 || !src_2 || !dst)
		return ffsc (__func__);
	
	i32 left_x = higher_of_i32 (src_1->left_x, src_2->left_x);
	i32 x_off = lower_of_i32 (src_1->left_x + src_1->width, src_2->left_x + src_2->width);
	i32 top_y = higher_of_i32 (src_1->top_y, src_2->top_y);
	i32 y_off = lower_of_i32 (src_1->top_y + src_1->height, src_2->top_y + src_2->height);
	
	dst->left_x = left_x;
	dst->top_y = top_y;
	
	if (x_off < left_x || y_off < top_y)
	{
		dst->width = 0;
		dst->height = 0;
		return FALSE;
	}
	else
	{
		dst->width = x_off - left_x;
		dst->height = y_off - top_y;
		return (x_off > left_x && y_off > top_y);
	}
}

static always_inline void Rect_adjust_size (Rect *r, i32 delta_size)
{
	if (!r || 2 * delta_size < -(r->width) || 2 * delta_size < -(r->height))
		return (void) ffsc (__func__);
	
	r->left_x -= delta_size;
	r->top_y -= delta_size;
	r->width += (2 * delta_size);
	r->height += (2 * delta_size);
}
