#define RANDOM_STANDARD_SEED_0 0x19803c70561f8414ull
#define RANDOM_STANDARD_SEED_1 0xcaca61eeae213995ull

typedef struct
{
	u64 state_0;
	u64 state_1;
} RandomState;

static never_inline void Random_set_seed_raw (RandomState *rs, u64 seed_0, u64 seed_1)
{
	if (!rs || (seed_0 == 0 && seed_1 == 0))
		return (void) ffsc (__func__);
	
	rs->state_0 = seed_0;
	rs->state_1 = seed_1;
}

static never_inline void Random_set_seed (RandomState *rs, u64 seed)
{
	if (!rs)
		return (void) ffsc (__func__);
	
	Random_set_seed_raw (rs, RANDOM_STANDARD_SEED_0 ^ (seed * (u64) 13), RANDOM_STANDARD_SEED_1 ^ (seed * (u64) 19));
}

static never_inline void Random_set_std_seed (RandomState *rs)
{
	if (!rs)
		return (void) ffsc (__func__);
	
	Random_set_seed_raw (rs, RANDOM_STANDARD_SEED_0, RANDOM_STANDARD_SEED_1);
}

// This is xoroshiro128+ 1.0, a public domain PRNG by David Blackman and Sabastiano Vigna
static always_inline u64 Random_get (RandomState *rs)
{
	u64 s0 = rs->state_0;
	u64 s1 = rs->state_1;
	u64 result = s0 + s1;
	s1 ^= s0;
	
	rs->state_0 = rotate_left_u64 (s0, 24) ^ s1 ^ (s1 << 16);
	rs->state_1 = rotate_left_u64 (s1, 37);
	return result;
}

// Assign to dst the state src would have if used to generate 2^64 random numbers. src and dst may point to the same object
static never_inline void Random_jump (const RandomState *src, RandomState *dst)
{
	if (!src || !dst)
		return (void) ffsc (__func__);
	
	u64 mask [2];
	mask [0] = 0xdf900294d8f554a5ull;
	mask [1] = 0x170865df4b3201fcull;
	
	RandomState tmp;
	Random_set_seed_raw (&tmp, src->state_0, src->state_1);
	
	dst->state_0 = 0;
	dst->state_1 = 0;
	
	i32 word_ix;
	i32 bit_ix;
	for (word_ix = 0; word_ix < 2; word_ix++)
		for (bit_ix = 0; bit_ix < 64; bit_ix++)
		{
			if (get_bit_u64 (mask [word_ix], bit_ix))
			{
				dst->state_0 ^= tmp.state_0;
				dst->state_1 ^= tmp.state_1;
			}
			(void) Random_get (&tmp);
		}
}
