#pragma once
#define GJK_MAX_ITTERATIONS 20
#define EPA_MAX_ITTERATIONS 50
#define EPA_MIN_DIFFERENCE 0.001f

#include "bear_shape.h"

struct Simplex
{
	uint32 num_points;
	Vec3f points[4]; // Assumes 3D
};

struct Overlap
{
	float32 depth;
	Vec3f normal;
	Vec3f point;
};

struct Body 
{
	Vec3f position;
	Vec3f velocity;
	Vec3f force;

	Shape shape;
};

// Ranges in a tightly packed array.
// Actual bodies stored in the "fancy array structure".

struct Physics
{
	Body bodies[2];
};

