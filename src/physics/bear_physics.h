#pragma once
#define GJK_MAX_ITTERATIONS 20
#define EPA_MAX_ITTERATIONS 50
#define EPA_MIN_DIFFERENCE 0.001f

#include "bear_shape.h"

struct SimplexPoint
{
	Vec3f point;
	Vec3f a, b;

	bool operator== (SimplexPoint o)
	{
		return point == o.point;
	}
};

struct Simplex
{
	uint32 num_points;
	SimplexPoint points[4]; // Assumes 3D (should 5 for 4D)
};

struct Overlap
{
	float32 depth;
	Vec3f normal;
	Vec3f shortest_translation;
	Vec3f contact_point;
};

struct CTransform
{
	COMPONENT;

	union
	{
		Transform transform;
		struct 
		{
			Vec3f pos;
			Vec3f scale;
			Quat rot;
		};
	};
};

struct CBody
{
	COMPONENT;

	float32 mass;
	float32 drag;

	Vec3f force;
	Vec3f velocity;

	Vec3f rotational_force;
	Vec3f rotational_velocity;

	Shape shape;
};

// Ranges in a tightly packed array.
// Actual bodies stored in the "fancy array structure".

enum BodyLimitFlags
{
	BLF_STATIC,
};

struct BodyLimit
{
	float32 min_limit;
	float32 max_limit;

	uint32 flags;
	EntityID owner;
};

struct Physics
{
	// Vec3f sort_direction; // Used for Temporal optimizations.
	Array<BodyLimit> body_limits;
};

bool add_body(Physics *phy, EntityID owner);

