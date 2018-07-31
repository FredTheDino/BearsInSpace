#include "bear_physics.h"
#include "bear_shape.cpp"
#include "bear_gjk.h"
#include "bear_epa.h"

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
		Vec3f a = simplex.points[0].point;
		Vec3f b = simplex.points[1].point;
		Vec3f c = simplex.points[2].point;
		Vec3f d = simplex.points[3].point;
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
	overlap.shortest_translation = overlap.normal * overlap.depth;

	if (debug_draw)
	{
		for (int32 i = 0; i < size(triangles); i++)
		{
			Triangle t = triangles[i];
			GFX::debug_draw_line(t.a.point, t.b.point, {0.9f, 0.9f, 0.3f});
			GFX::debug_draw_line(t.a.point, t.c.point, {0.9f, 0.9f, 0.3f});
			GFX::debug_draw_line(t.c.point, t.b.point, {0.9f, 0.9f, 0.3f});
			Vec3f center = (t.a.point + t.b.point + t.c.point) / 3.0f;
			GFX::debug_draw_line(center, center + t.normal, {0.5f, 0.3f, 0.75f});
		}

		GFX::debug_draw_line({}, overlap.shortest_translation, {1.0f, 0.5f, 0.3f});
		GFX::debug_draw_point(overlap.contact_point, {0.5f, 1.0f, 0.3f});
		GFX::debug_draw_point(overlap.contact_point, {1.0f, 1.0f, 1.0f});
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

Vec3f corner(Vec3f dim, Transform t, float32 x, float32 y, float32 z)
{
	Vec3f box_point =
		{
			dim.x * (x * 0.5f), 
			dim.y * (y * 0.5f), 
			dim.z * (z * 0.5f)
		};
	return t * box_point;
}

void debug_draw_box(Transform t, Vec3f dim, Vec3f color)
{
	GFX::debug_draw_line(corner(dim, t, -1, -1, -1), corner(dim, t, -1, -1,  1), color);
	GFX::debug_draw_line(corner(dim, t, -1, -1, -1), corner(dim, t, -1,  1, -1), color);
	GFX::debug_draw_line(corner(dim, t, -1, -1, -1), corner(dim, t,  1, -1, -1), color);

	GFX::debug_draw_line(corner(dim, t, -1, -1,  1), corner(dim, t,  1, -1,  1), color);
	GFX::debug_draw_line(corner(dim, t, -1, -1,  1), corner(dim, t, -1,  1,  1), color);

	GFX::debug_draw_line(corner(dim, t, -1,  1, -1), corner(dim, t, -1,  1,  1), color);
	GFX::debug_draw_line(corner(dim, t, -1,  1, -1), corner(dim, t,  1,  1, -1), color);

	GFX::debug_draw_line(corner(dim, t,  1, -1, -1), corner(dim, t,  1, -1,  1), color);
	GFX::debug_draw_line(corner(dim, t,  1, -1, -1), corner(dim, t,  1,  1, -1), color);

	GFX::debug_draw_line(corner(dim, t,  1,  1,  1), corner(dim, t,  1,  1, -1), color);
	GFX::debug_draw_line(corner(dim, t,  1,  1,  1), corner(dim, t,  1, -1,  1), color);
	GFX::debug_draw_line(corner(dim, t,  1,  1,  1), corner(dim, t, -1,  1,  1), color);
}

void debug_draw_box(Shape s, Vec3f color)
{
	debug_draw_box(s.transform, {s.width, s.height, s.depth}, color);
}

void debug_draw_sphere(Transform t, float32 radius, Vec3f color)
{
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
			current_loop[j] = t * (point * radius);

			if (i == 0) continue;
			GFX::debug_draw_line(last_loop[j], current_loop[j], color);
			if (j > 0)
				GFX::debug_draw_line(current_loop[j], current_loop[j - 1], color);
		}
	}

}

void debug_draw_sphere(Shape s, Vec3f color)
{
	debug_draw_sphere(s.transform, s.radius, color);
}

bool add_body(Physics *phy, EntityID owner)
{
	BodyLimit limit;
	limit.owner = owner;
	append(&phy->body_limits, limit);
	return true;
}

// System
void update_physics(ECS *ecs, Physics *engine, float32 delta)
{
	float64 t = world->clk.time;
	t = t * 0.5f;

	Shape shape_a = make_sphere(4.0f);
	Shape shape_b = make_box(10.0f, 1.0f, 10.0f);

	GFX::debug_draw_line({}, {5.0f, 0.0f, 0.0f}, {1.0f, 0.0f, 0.0f});
	GFX::debug_draw_line({}, {0.0f, 5.0f, 0.0f}, {0.0f, 1.0f, 0.0f});
	GFX::debug_draw_line({}, {0.0f, 0.0f, 5.0f}, {0.0f, 0.0f, 1.0f});

	// Fixed physics steps? Maybe.
	// NOTE: Should we extend this to use 3 axies, so we create bounding boxes
	// for all the objects. How much of a speed up will it give? Is this fast
	// enough for now? Probably.
	Vec3f gravity = {0.0f, -10.0f * delta, 0.0f};
	Vec3f sort_direction = {0.0f, 0.0f, 1.0f};

	//   Simulate the velocty change
	//   Update the limits
	for (uint32 i = 0; i < size(engine->body_limits); i++)
	{
		BodyLimit limit = get(engine->body_limits, i);
		
		CBody *body = (CBody *) get_component(ecs, limit.owner, C_BODY);
		CTransform *transform = (CTransform *) get_component(ecs, limit.owner, C_TRANSFORM);;

		body->velocity += body->force;
		if (body->mass != 0.0f)
			body->velocity += gravity;
		body->force = {};

		// TODO: This should in the end be the whole transform.
		// TODO: Maybe I should change to a "max pos" and a "min pos"
		// in the shape that I then apply the transform to. But that would
		// not work with rotations. So maybe just a "limit sphere".
		// Cause that would be usefull for range test just before the
		// main collision test.
		
		body->shape.transform = transform->transform;
		Quat rot_vel = 
			toQ(
				body->rotational_velocity.x * delta, 
				body->rotational_velocity.y * delta, 
				body->rotational_velocity.z * delta);
		body->shape.transform.rot *= rot_vel;


		body->shape.transform.rot = normalize(body->shape.transform.rot);

		body->shape.transform.rot = body->shape.transform.rot;
		body->shape.transform.pos += body->velocity * delta;


		// TODO: Might need to do 2 functions. So I don't have to dot it twice.
		// But it is fairly cheep so I'll leave it like this.
		Transform t = body->shape.transform;
		float32 velocity_scale = dot(sort_direction, body->velocity);
		limit.min_limit = 
			minimum(velocity_scale, 0.0f) + 
			dot(t * support(t.rot / -sort_direction, body->shape), sort_direction);

		limit.max_limit = 
			maximum(velocity_scale, 0.0f) + 
			dot(t * support(t.rot / sort_direction, body->shape), sort_direction);

		ASSERT(limit.min_limit < limit.max_limit);

		set(engine->body_limits, i, limit);
	}

	// TODO: Drag the shapes by the velocity. It will help tunneling a bit.

	// Insertion sort is one of the fastest with allready sorted lists.
	// If you don't randomly change the sorting direction every frame,
	// this will be VERY performant.
	for (int32 i = 1; i < size(engine->body_limits); i++)
	{
		BodyLimit i_limit = engine->body_limits[i];
		for (int32 j = i - 1; 0 <= j; j--)
		{
			if (i_limit.min_limit < engine->body_limits[j].min_limit)
			{
				for (uint32 copy_index = i; copy_index > j; copy_index--)
				{
					set(engine->body_limits, copy_index, engine->body_limits[copy_index - 1]);
				}
				set(engine->body_limits, j, i_limit);
				break;
			}
		}
	}

	for (int32 outer_limit_index = 0; 
		outer_limit_index < size(engine->body_limits);
		outer_limit_index++)
	{
		BodyLimit outer_limit = engine->body_limits[outer_limit_index];
		CBody *outer = (CBody *) get_component(ecs, outer_limit.owner, C_BODY);
		Vec3f offset = {(float32) outer_limit_index, 1.0f, 1.0f};
		GFX::debug_draw_line(
				sort_direction * outer_limit.min_limit + offset, 
				sort_direction * outer_limit.max_limit + offset, 
				{1.0f, 0.32f, 0.77f});
		switch(outer->shape.id)
		{
			case (SHAPE_BOX):
				debug_draw_box(outer->shape, {1.0f, 1.0f, 0.0f});
				break;
			case (SHAPE_SPHERE):
				debug_draw_sphere(outer->shape, {1.0f, 1.0f, 0.0f});
				break;
			default:
				GFX::debug_draw_point(outer->shape.transform.pos, {1.0f, 1.0f, 0.0f});

		}
		for (int32 inner_limit_index = outer_limit_index + 1; 
				inner_limit_index < size(engine->body_limits);
				inner_limit_index++)
		{
			BodyLimit inner_limit = engine->body_limits[inner_limit_index];
			if (outer_limit.max_limit < inner_limit.min_limit)
				break; // They can't possibly collide.
			CBody *inner = (CBody *) get_component(ecs, inner_limit.owner, C_BODY);

			if (outer->mass == 0.0f && inner->mass == 0.0f)
				continue;


			Shape a_shape = inner->shape;
			Shape b_shape = outer->shape;
			Overlap overlap = overlap_test({0.0f, 0.0f, 1.0f}, a_shape, b_shape, false);
			if (overlap.depth <= 0)
			{
				continue;
			}
			// Static VS Dynamic

			float32 bounce = 0.5f;
			// Do we need this? It is needed for calculations. But I don't know if we need it.
			Vec3f temp_inertia = {3.0f, 3.0f, 3.0f};

			Vec3f inner_radius = overlap.contact_point - inner->shape.transform.pos;
			Vec3f outer_radius = overlap.contact_point - outer->shape.transform.pos;
			Vec3f inner_velocity = inner->velocity + cross(inner_radius, overlap.normal);
			Vec3f outer_velocity = outer->velocity + cross(outer_radius, overlap.normal);
			float32 relative_velocity = dot(inner_velocity - outer_velocity, overlap.normal) * (1.0f + bounce);
			float32 inv_outer_mass = outer->mass == 0.0f ? 0.0f : 1.0f / outer->mass;
			float32 inv_inner_mass = inner->mass == 0.0f ? 0.0f : 1.0f / inner->mass;

			float32 total_mass = inner->mass + outer->mass;
			outer->shape.transform.pos += overlap.normal * overlap.depth * inv_outer_mass / total_mass;
			inner->shape.transform.pos -= overlap.normal * overlap.depth * inv_inner_mass / total_mass;

			if (relative_velocity < 0.0f)
				continue;

			// TODO
			float32 j = -relative_velocity / (inv_outer_mass + inv_inner_mass +
				dot(overlap.normal * dot(cross(inner_radius, overlap.normal), temp_inertia), inner_radius) +
				dot(overlap.normal * dot(cross(inner_radius, overlap.normal), temp_inertia), inner_radius));

			GFX::debug_draw_line(overlap.contact_point, overlap.contact_point + overlap.normal * j * 10.0f, {1.0f, 0.0f, 1.0f});
			
			outer->velocity -= overlap.normal * j * inv_outer_mass;
			inner->velocity += overlap.normal * j * inv_inner_mass;
			// TODO: Rotations aren't done correctly. The forces needs to be rotated since 
			// they're ino world coordinates. This is a major bug.
			outer->rotational_velocity -= cross(outer_radius, overlap.normal * j) * 1.0f * inv_outer_mass;
			inner->rotational_velocity += cross(inner_radius, overlap.normal * j) * 1.0f * inv_inner_mass;
		}
	}

	for (uint32 i = 0; i < size(engine->body_limits); i++)
	{
		BodyLimit limit = get(engine->body_limits, i);
		CBody *body = (CBody *) get_component(ecs, limit.owner, C_BODY);
		CTransform *component = (CTransform *) get_component(ecs, limit.owner, C_TRANSFORM);;
		component->transform = body->shape.transform;
	}
}
