//
// *Really* minimal PCG32 code, stolen from the internet.
// http://www.pcg-random.org/download.html
//

//
// NOTE: This is only 32 bits of random numbers,
// it can be expanded to 64 bits of random.
//

#define MAX_RANDOM_INT (0xFFFFFFFF)

struct RandomState { 
	uint64 state;  
	uint64 inc; 
};

RandomState seed(uint64 i)
{

	RandomState rng;
	rng.state = i * 87654291002987654ULL;
	rng.inc = i * 198765777234562234ULL;
	return rng;
}

uint32 random(RandomState *rng)
{
    uint64 oldstate = rng->state;
    rng->state = oldstate * 6364136223846793005ULL + (rng->inc|1);
    uint32 xorshifted = ((oldstate >> 18u) ^ oldstate) >> 27u;
    uint32 rot = oldstate >> 59u;
    return (xorshifted >> rot) | (xorshifted << ((-rot) & 31));
}

float32 random_float(RandomState *rng)
{
	float32 r = random(rng) / (float32) MAX_RANDOM_INT;
	return r;
}

int32 random_int_in_range(RandomState *rng, int32 min, int32 max)
{
	int32 range = max - min;
	int32 rand_range = random(rng) % range;
	return min + rand_range;
}

float32 random_float_in_range(RandomState *rng, float32 min, float32 max)
{
	float32 range = max - min;
	float32 rand_range = random_float(rng) * range;
	return min + rand_range;
}

Vec2f random_unit_vec2f(RandomState *rng)
{
	float32 angle = random_float(rng) * 2 * PI;
	Vec2f result = {cos(angle), sin(angle)};
	return result;
}

Vec3f random_unit_vec3f(RandomState *rng)
{
	float32 alpha = random_float(rng) * 2.0f * PI;
	float32 beta  = random_float(rng) * 2.0f * PI;

	float32 sa = sin(alpha);
	float32 ca = cos(alpha);
	float32 sb = sin(beta);
	float32 cb = cos(beta);
	
	Vec3f result = {
		(float32) (sa * cb),
		(float32) (ca),
		(float32) (sa * sb)
	};
	return result;
}


