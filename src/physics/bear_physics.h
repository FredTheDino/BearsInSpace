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

#include "bear_gjk.h"
#include "bear_epa.h"

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

Overlap overlap_test(Vec3f inital_direction, 
		Shape a_shape,
		Shape b_shape,
		bool debug_draw=false)
{
	// TODO: Transforms that work.
	Simplex simplex = gjk(inital_direction, a_shape, b_shape);
	if (simplex.num_points != 4)
		return {};

	if (debug_draw)
	{
		Vec3f a = simplex.points[0];
		Vec3f b = simplex.points[1];
		Vec3f c = simplex.points[2];
		Vec3f d = simplex.points[3];
		Vec3f center = (a + b + c + d) / 4.0f;

		GFX::debug_draw_line(a, b, {0.3f, 0.9f, 0.9f});
		GFX::debug_draw_line(a, c, {0.3f, 0.9f, 0.9f});
		GFX::debug_draw_line(a, d, {0.3f, 0.9f, 0.9f});
		GFX::debug_draw_line(b, c, {0.3f, 0.9f, 0.9f});
		GFX::debug_draw_line(b, d, {0.3f, 0.9f, 0.9f});
		GFX::debug_draw_line(c, d, {0.3f, 0.9f, 0.9f});
		GFX::debug_draw_point(center, {0.0f, 1.0f, 0.5f});
	}

	Array<Triangle> triangles = create_array<Triangle>(10);
	Overlap overlap = epa(simplex, &triangles, a_shape, b_shape);

	if (debug_draw)
	{
		for (int32 i = 0; i < size(triangles); i++)
		{
			Triangle t = triangles[i];
			GFX::debug_draw_line(t.a, t.b, {0.9f, 0.9f, 0.3f});
			GFX::debug_draw_line(t.a, t.c, {0.9f, 0.9f, 0.3f});
			GFX::debug_draw_line(t.c, t.b, {0.9f, 0.9f, 0.3f});
			Vec3f center = (t.a + t.b + t.c) / 3.0f;
			GFX::debug_draw_line(center, center + t.normal, {0.5f, 0.3f, 0.75f});
		}

		GFX::debug_draw_line({}, overlap.point, {1.0f, 0.5f, 0.3f});
	}

	delete_array(&triangles);

	return overlap;
}

struct BodyComponent
{
	COMPONENT;
	//Transform transform;
	Shape shape;
};

Vec3f corner(Vec3f dim, Vec3f pos, float32 x, float32 y, float32 z)
{
	return
		{
			dim.x * (x * 0.5f) + pos.x, 
			dim.y * (y * 0.5f) + pos.y, 
			dim.z * (z * 0.5f) + pos.z
		};
}

void debug_draw_box(Shape shape, Vec3f color)
{
	ASSERT(shape.id == SHAPE_BOX);
	Vec3f dim = {shape.width, shape.height, shape.depth};
	Vec3f pos = shape.position;

	GFX::debug_draw_line(corner(dim, pos, -1, -1, -1), corner(dim, pos, -1, -1,  1), color);
	GFX::debug_draw_line(corner(dim, pos, -1, -1, -1), corner(dim, pos, -1,  1, -1), color);
	GFX::debug_draw_line(corner(dim, pos, -1, -1, -1), corner(dim, pos,  1, -1, -1), color);

	GFX::debug_draw_line(corner(dim, pos, -1, -1,  1), corner(dim, pos,  1, -1,  1), color);
	GFX::debug_draw_line(corner(dim, pos, -1, -1,  1), corner(dim, pos, -1,  1,  1), color);

	GFX::debug_draw_line(corner(dim, pos, -1,  1, -1), corner(dim, pos, -1,  1,  1), color);
	GFX::debug_draw_line(corner(dim, pos, -1,  1, -1), corner(dim, pos,  1,  1, -1), color);

	GFX::debug_draw_line(corner(dim, pos,  1, -1, -1), corner(dim, pos,  1, -1,  1), color);
	GFX::debug_draw_line(corner(dim, pos,  1, -1, -1), corner(dim, pos,  1,  1, -1), color);

	GFX::debug_draw_line(corner(dim, pos,  1,  1,  1), corner(dim, pos,  1,  1, -1), color);
	GFX::debug_draw_line(corner(dim, pos,  1,  1,  1), corner(dim, pos,  1, -1,  1), color);
	GFX::debug_draw_line(corner(dim, pos,  1,  1,  1), corner(dim, pos, -1,  1,  1), color);
}

void debug_draw_sphere(Shape shape, Vec3f color)
{
	ASSERT(shape.id == SHAPE_SPHERE);
	float32 radius = shape.radius;
	Vec3f pos = shape.position;

#define NUM_SPHERE_SEGMENTS 16
	Vec3f last_loop[NUM_SPHERE_SEGMENTS * 2];
	Vec3f current_loop[NUM_SPHERE_SEGMENTS * 2];

	for (int32 i = 0; i < NUM_SPHERE_SEGMENTS; i++)
	{
		for (int32 j = 0; j < NUM_SPHERE_SEGMENTS; j++)
		{
			float32 alpha = (i) * 2.0f * PI / NUM_SPHERE_SEGMENTS; // Vertical
			float32 beta  = (j) * 2.0f * PI / NUM_SPHERE_SEGMENTS; // Horizontal

			last_loop[j] = current_loop[j];
			Vec3f point = {
				(float32) (sin(alpha) * cos(beta)),
				(float32) (cos(alpha)),
				(float32) (sin(alpha) * sin(beta))
			};
			current_loop[j] = point * radius + pos;

			if (i == 0) continue;
			GFX::debug_draw_line(last_loop[j], current_loop[j], color);
			if (j > 0)
				GFX::debug_draw_line(current_loop[j], current_loop[j - 1], color);
		}
	}

}

// System
void update_physics(Physics *engine, float32 delta)
{
	float64 t = world->clk.time;
	t = t * 0.5f;

	Shape shape_a = make_sphere(4.0f);
	Shape shape_b = make_box(5.0f, 5.0f, 5.0f);

	GFX::debug_draw_line({}, {1.0f, 0.0f, 0.0f}, {1.0f, 0.0f, 0.0f});
	GFX::debug_draw_line({}, {0.0f, 1.0f, 0.0f}, {0.0f, 1.0f, 0.0f});
	GFX::debug_draw_line({}, {0.0f, 0.0f, 1.0f}, {0.0f, 0.0f, 1.0f});

	Vec3f offset = {14.0f, 0.0f, 0.0f};

	shape_a.position = {};
	shape_b.position = {sinf(t) * 3.5f, cosf(t) * 4.0f, cosf(t * 0.33333) * 3.0f};
	shape_a.position += offset;
	shape_b.position += offset;

	debug_draw_sphere(shape_a, {0.2f, 0.0f, 0.2f});

	Overlap overlap = overlap_test({0.0f, 0.0f, 1.0f}, shape_a, shape_b, true);
	if (overlap.depth == 0)
	{
		debug_draw_sphere(shape_a, {0.0f, 0.0f, 1.0f});
		debug_draw_box(shape_b, {0.0f, 0.0f, 1.0f});
	}
	else
	{
		debug_draw_sphere(shape_a, {1.0f, 0.0f, 0.0f});
		debug_draw_box(shape_b, {0.0f, 1.0f, 0.0f});
		shape_a.position -= overlap.point;
		debug_draw_sphere(shape_a, {1.0f, 1.0f, 0.0f});
	}
}
