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

struct CTransform
{
	COMPONENT;

	union
	{
		Transform transform;
		struct 
		{
			Vec3f position;
			Vec3f scale;
			Quat orientation;
		};
	};
};

struct CBody
{
	COMPONENT;

	float32 restitution;

	Mat4f inverse_inertia;

	float32 inverse_mass;
	float32 linear_damping; 
	float32 angular_damping; 
	
	// A pointer so we don't have to go looking for the
	// transform when updating the body. Not garanteed to work
	// outside of the physics step.
	Transform *_transform; 


	Vec3f velocity; // Linear velocity
	Vec3f acceleration; // Linear accelration
	Vec3f force_accumulator;

	// Stored as axis angles.
	Vec3f rotation;
	Vec3f torque_accumulator;

	Shape shape;
};

void set_mass(CBody *body, float32 mass)
{
	if (mass == 0.0f)
		body->inverse_mass = mass;
	else
		body->inverse_mass = 1.0f / mass;
}

struct Collision
{
	float32 depth;
	Vec3f normal;
	Vec3f contact_point;

	CBody *a;
	CBody *b;
};

// Ranges in a tightly packed array.
// Actual bodies stored in the "fancy array structure".

struct BodyLimit
{
	float32 min_limit;
	float32 max_limit;

	EntityID owner;
};

struct Physics
{
	// Vec3f sort_direction; // Used for Temporal optimizations.
	Array<BodyLimit> body_limits;
	Array<Collision> collisions;
};

bool add_body(Physics *phy, EntityID owner);

