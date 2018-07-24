#define GJK_MAX_ITTERATIONS 20
#define EPA_MAX_ITTERATIONS 20
#define EPA_MIN_DIFFERENCE 0.00001f

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

struct Physics
{
	
};

Overlap overlap_test(Vec3f inital_direction, 
		Shape a_shape,
		Shape b_shape,
		bool debug_draw=false)
{
	// TODO: Transforms that work.
	Simplex simplex = gjk(inital_direction, a_shape, b_shape);

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
	Shape *shape;
};

// System
void update_physics(Physics *engine, float32 delta)
{
	float64 t = world->clk.time;
	t = t * 0.5f;
	Shape s_a = make_line({0.0f, 0.0f, 1.0f}, {-1.0f, 0.0f, 0.0f});
	Shape s_b = make_box(3, 2, 1);

	Shape shape_a = make_sum(&s_a, &s_b);
	Shape shape_b = make_sphere(5.0f);
	shape_b.position = {sinf(t) * 4.0f, cosf(t) * 5.0f, sinf(PI * t) * 0.1f};
	Overlap overlap = overlap_test({0.0f, 0.0f, 1.0f}, shape_a, shape_b, true);

}
