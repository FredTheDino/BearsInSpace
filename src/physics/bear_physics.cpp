#include "bear_physics.h"
#include "bear_shape.cpp"
#include "bear_gjk.h"
#include "bear_epa.h"

Collision collision_test(Vec3f inital_direction, 
		CBody *a,
		CBody *b,
		bool debug_draw=false)
{
	// TODO: Transforms that work.
	Simplex simplex = gjk(inital_direction, a->shape, *a->_transform, b->shape, *b->_transform);
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
	Collision collision = epa(simplex, &triangles, a->shape, *a->_transform, b->shape, *b->_transform);
	collision.a = a;
	collision.b = b;
	collision.shortest_translation = collision.normal * collision.depth;

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

		GFX::debug_draw_line({}, collision.shortest_translation, {1.0f, 0.5f, 0.3f});
		GFX::debug_draw_point(collision.contact_point, {0.5f, 1.0f, 0.3f});
		GFX::debug_draw_point(collision.contact_point, {1.0f, 1.0f, 1.0f});
	}

	delete_array(&triangles);

	return collision;
}

struct BodyComponent
{
	COMPONENT;
	//Transform transform;
	Shape shape;
};

Mat4f calculate_inertia_tensor(Shape shape, float32 mass)
{
	if (mass == 0.0f)
		return {};

	Mat4f inertia = {};
	inertia._33 = 1.0f;

	switch (shape.id)
	{
		case (SHAPE_BOX):
			{
				float32 c = mass / 12.0f;
				float32 h2 = (shape.height / 2) * (shape.height / 2);
				float32 w2 = (shape.width / 2) * (shape.width / 2);
				float32 d2 = (shape.depth / 2) * (shape.depth / 2);
				inertia._00 = c * (h2 + d2);
				inertia._11 = c * (w2 + d2);
				inertia._22 = c * (w2 + h2);
			}
			break;
		case (SHAPE_SPHERE):
			{
				float32 c = 2 * mass / 5.0f; // Sold sphere. Use 2 / 3 for shell.
				inertia._00 = c;
				inertia._11 = c;
				inertia._22 = c;
			}
			break;
		default:
			break;
	}

	return inertia;
}

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

void debug_draw_box(CBody *body, Vec3f color)
{
	Vec3f dim = {body->shape.width, body->shape.height, body->shape.depth};
	debug_draw_box(*body->_transform, dim, color);
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

void debug_draw_sphere(CBody *body, Vec3f color)
{
	debug_draw_sphere(*body->_transform, body->shape.radius, color);
}

bool add_body(Physics *phy, EntityID owner)
{
	BodyLimit limit;
	limit.owner = owner;
	append(&phy->body_limits, limit);
	return true;
}

void integrate_body(CBody *body, Vec3f gravity, float32 delta)
{
	body->acceleration = {};
	body->acceleration = body->force_accumulator * body->inverse_mass + gravity;

	body->velocity *= pow((float64) body->linear_damping, (float64) delta);

	body->velocity += body->acceleration * delta;
	body->_transform->position += body->velocity * delta + body->acceleration * delta * delta / 2.0f;

	body->rotation *= pow((float64) body->angular_damping, (float64) delta);
	Quat r = {body->rotation.x, body->rotation.y, body->rotation.z, 0.0f};
	Quat o = body->_transform->orientation;
	body->_transform->orientation = normalize(o + r * o * (delta / 2.0f));

	
}

BodyLimit find_limit(CBody *body, Vec3f sort_direction)
{
	Transform t = *body->_transform;
	BodyLimit limit;
	limit.owner = body->base.owner;

	float32 velocity_scale = dot(sort_direction, body->velocity);
	Vec3f lowest_point  = t * support(t.orientation / -sort_direction, body->shape);
	Vec3f highest_point = t * support(t.orientation / sort_direction, body->shape);
	limit.min_limit = minimum(0.0f, velocity_scale) + dot(lowest_point, sort_direction);
	limit.max_limit = maximum(0.0f, velocity_scale) + dot(highest_point, sort_direction);

	ASSERT(limit.min_limit < limit.max_limit);
	return limit;
}

void sort_limits(Array<BodyLimit> *_limits)
{
	// Insertion sort is one of the fastest with allready sorted lists.
	// If you don't randomly change the sorting direction every frame,
	// this will be VERY performant.
	Array<BodyLimit> limits = *_limits;
	for (int32 i = 1; i < size(limits); i++)
	{
		BodyLimit i_limit = get(limits, i);
		for (int32 j = i - 1; 0 <= j; j--)
		{
			if (i_limit.min_limit < get(limits, j).min_limit)
			{
				for (uint32 copy_index = i; copy_index > j; copy_index--)
				{
					set(limits, copy_index, limits[copy_index - 1]);
				}
				set(limits, j, i_limit);
				break;
			}
		}
	}
}

// TODO: Do a "NOT IMPLEMENTED MACRO" that prevents you from compiling.
void find_collisions(ECS *ecs, Physics *engine)
{
	for (int32 outer_limit_index = 0; 
		outer_limit_index < size(engine->body_limits);
		outer_limit_index++)
	{
		BodyLimit outer_limit = engine->body_limits[outer_limit_index];
		CBody *a = (CBody *) get_component(ecs, outer_limit.owner, C_BODY);
		for (int32 inner_limit_index = outer_limit_index + 1; 
				inner_limit_index < size(engine->body_limits);
				inner_limit_index++)
		{
			BodyLimit inner_limit = engine->body_limits[inner_limit_index];
			if (outer_limit.max_limit < inner_limit.min_limit)
				break; // They can't possibly collide.
			CBody *b = (CBody *) get_component(ecs, inner_limit.owner, C_BODY);

			if (a->inverse_mass == 0.0f && b->inverse_mass == 0.0f)
				continue;

			Collision collision = collision_test({0.0f, 0.0f, 1.0f}, a, b, false);
			if (0.0f < collision.depth)
			{
				append(&engine->collisions, collision);
			}
		}
	}
}

Mat4f make_orthonormal_basis(Vec3f x)
{
	// NOTE: You can make this go faster. But I don't think we need to.
	Vec3f y, z;
	// NOTE: This is perty much garanteed to work.
	if (absolute(x.x) > absolute(x.y))
	{
		y = {0.0f, 1.0f, 0.0f};
	}
	else
	{
		y = {1.0f, 0.0f, 0.0f};
	}

	z = cross(x, y);
	z = normalize(z);
	y = cross(z, x);

	return 
	{
		x.x, y.x, z.x, .0f,
		x.y, y.y, z.y, .0f, 
		x.z, y.z, z.z, .0f,
		.0f, .0f, .0f, 1.0f
	};
}

void solve_collisions_randomly(Physics *engine)
{
	// Solves them in the order they come in. For now.
	auto collisions = engine->collisions;
	for (int32 i = 0; i < size(collisions); i++)
	{
		DEBUG_LOG("HIT!\n");
		Collision collision = get(collisions, i);
		CBody *a = collision.a;
		CBody *b = collision.b;

		// Shouldn't this inverse inertia be rotated by the the bodys rotation relative to the normal? Maybe it doesn't? How do I figure it out?
		// TODO: Move the bodies out of eachother.
		float32 float_correct = 0.0001f;
		if (a->inverse_mass == 0.0f)
			b->_transform->position += collision.normal * (collision.depth + float_correct);
		else if (b->inverse_mass == 0.0f)
			a->_transform->position -= collision.normal * (collision.depth + float_correct);
		Vec3f impulse;
		{
			Mat4f to_world = make_orthonormal_basis(collision.normal);
			Mat4f to_contact = transpose(to_world); // This works cause it's a rotation.
			float32 bounce = 0.0f;
			float32 delta_vel;
			Vec3f vel;
			if (a)
			{
				Vec3f a_rel_position = collision.contact_point - a->_transform->position;
				Vec3f rotation_per_impulse = a->inverse_inertia * cross(a_rel_position, collision.normal);
				Vec3f velocity_per_impulse = cross(rotation_per_impulse, a_rel_position);
				// TODO: This only works because we only care for the x component. Won't work for friction.
				delta_vel = dot(collision.normal, velocity_per_impulse); 
				delta_vel += a->inverse_mass;
				vel = a->velocity + cross(a->rotation, a_rel_position);
			}


			if (b)
			{
				Vec3f b_rel_position = collision.contact_point - b->_transform->position;
				Vec3f rotation_per_impulse = b->inverse_inertia * cross(b_rel_position, collision.normal);
				Vec3f velocity_per_impulse = cross(rotation_per_impulse, b_rel_position);

				// TODO: This only works because we only care for the x component. Won't work for friction.
				delta_vel += dot(collision.normal, velocity_per_impulse);
				delta_vel += b->inverse_mass;

				vel += b->velocity + cross(b->rotation, b_rel_position);
			}

			// TODO: This only works because we only care for the x component. Won't work for friction.
			float32 contact_speed = dot(collision.normal, vel);
			float32 impulse_strength = contact_speed * (1.0f + bounce) / delta_vel; // Supposed to be anegative here.
			/*
			if (impulse_strength > 0.0f)
				continue;
			*/
#if 0
			Vec3f impulse_contact;
			impulse_contact.x = impulse_strength;
			impulse_contact.y = {};
			impulse_contact.z = {};

			impulse = to_world * impulse_contact;
#else
			impulse = collision.normal * impulse_strength;
		}
#endif

		// Update velocity
		a->velocity += impulse * a->inverse_mass;
		b->velocity -= impulse * b->inverse_mass;

		Vec3f a_rel_position = collision.contact_point - a->_transform->position;
		Vec3f b_rel_position = collision.contact_point - b->_transform->position;
		a->rotation += a->inverse_inertia * cross(impulse, a_rel_position);
		b->rotation -= b->inverse_inertia * cross(impulse, b_rel_position);

	}
}

void debug_draw_engine(ECS *ecs, Physics *engine)
{
	Vec3f sort_direction = {0.0f, 0.0f, 1.0f}; // COpY

	GFX::debug_draw_line({}, {5.0f, 0.0f, 0.0f}, {1.0f, 0.0f, 0.0f});
	GFX::debug_draw_line({}, {0.0f, 5.0f, 0.0f}, {0.0f, 1.0f, 0.0f});
	GFX::debug_draw_line({}, {0.0f, 0.0f, 5.0f}, {0.0f, 0.0f, 1.0f});

	uint32 grid_dim = 8;
	for (uint32 grid = 1; grid < grid_dim * 2.0f; grid++)
	{
		float32 spacing = 1.0f;
		Vec3f color = {0.3f, 0.3f, 0.4f};
		float32 start = -spacing * grid_dim;
		float32 length = spacing * grid_dim;
		float32 offset = start + grid * spacing;

		Vec3f z_start = {-length, 0.0f, offset};
		Vec3f z_end = {length, 0.0f, offset};
		GFX::debug_draw_line(z_start, z_end, color);

		Vec3f x_start = {offset, 0.0f, -length};
		Vec3f x_end = {offset, 0.0f, length};
		GFX::debug_draw_line(x_start, x_end, color);
	}

	for (uint32 i = 0; i < size(engine->body_limits); i++)
	{
		BodyLimit limit = get(engine->body_limits, i);
		CBody *body = (CBody *) get_component(ecs, limit.owner, C_BODY);

		Vec3f offset = {(float32) i, 1.0f, 1.0f};
		GFX::debug_draw_line(
				sort_direction * limit.min_limit + offset, 
				sort_direction * limit.max_limit + offset, 
				{1.0f, 0.32f, 0.77f});

		switch(body->shape.id)
		{
			case (SHAPE_BOX):
				debug_draw_box(body, {1.0f, 1.0f, 0.0f});
				break;
			case (SHAPE_SPHERE):
				debug_draw_sphere(body, {1.0f, 1.0f, 0.0f});
				break;
			default:
				GFX::debug_draw_point(body->_transform->position, {1.0f, 1.0f, 0.0f});
		}
	}
}

// System
void update_physics(ECS *ecs, Physics *engine, float32 delta)
{

	// Fixed physics steps? Maybe.
	// NOTE: Should we extend this to use 3 axies, so we create bounding boxes
	// for all the objects. How much of a speed up will it give? Is this fast
	// enough for now? Probably.
	Vec3f gravity = {0.0f, -10.0f * delta, 0.0f};
	Vec3f sort_direction = {0.0f, 0.0f, 1.0f};

	//   Simulate the veloctiy change
	//   Update the limits
	for (uint32 i = 0; i < size(engine->body_limits); i++)
	{
		BodyLimit limit = get(engine->body_limits, i);
		
		CBody *body = (CBody *) get_component(ecs, limit.owner, C_BODY);
		CTransform *c_transform = (CTransform *) get_component(ecs, limit.owner, C_TRANSFORM);
		body->_transform = &c_transform->transform;

		ASSERT(body);
		ASSERT(body->_transform);

		integrate_body(body, {0.0f, -10.0f * delta, 0.0f}, delta);

		BodyLimit limt = find_limit(body, sort_direction);
		set(engine->body_limits, i, limt);
	}

	// TODO: Drag the shapes by the velocity. It will help tunneling a bit.



	//sort_limits(&engine->body_limits);

	clear(&engine->collisions);
	find_collisions(ecs, engine);

	solve_collisions_randomly(engine);
#if 0
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

			Collision collision = overlap_test({0.0f, 0.0f, 1.0f}, outer, inner, false);
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
#endif
}
