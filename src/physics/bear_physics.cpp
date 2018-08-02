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
	if (collision.depth > 0.0f)
	{
		collision.a = a;
		collision.b = b;
		Vec3f penetration = collision.normal * collision.depth;

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

			GFX::debug_draw_line({}, penetration, {1.0f, 0.5f, 0.3f});
			GFX::debug_draw_point(collision.contact_point, {0.5f, 1.0f, 0.3f});
			GFX::debug_draw_point(collision.contact_point, {1.0f, 1.0f, 1.0f});
		}
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

Quat rotate_by_vector(Quat q, Vec3f v, float32 delta = 1.0f)
{
	Quat r = {v.x, v.y, v.z, 0.0f};
	Quat out = q + r * q * (delta / 2.0f);
	return normalize(out);
}

void integrate_body(CBody *body, Vec3f gravity, float32 delta)
{
	body->acceleration = {};
	if (body->inverse_mass != 0.0f)
		body->acceleration = body->force_accumulator * body->inverse_mass + gravity;

	body->velocity *= pow((float64) body->linear_damping, (float64) delta);

	body->velocity += body->acceleration * delta;
	body->_transform->position += body->velocity * delta + body->acceleration * delta * delta / 2.0f;

	body->rotation += body->torque_accumulator;
	body->rotation *= pow((float64) body->angular_damping, (float64) delta);
	body->_transform->orientation = rotate_by_vector(body->_transform->orientation, body->rotation, delta);
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
			if (isnan(collision.contact_point.x))
				continue;
			if (0.0f < collision.depth)
			{
				if (a->inverse_mass == 0.0f)
					collision.a = nullptr;
				if (b->inverse_mass == 0.0f)
					collision.b = nullptr;
				append(&engine->collisions, collision);
			}
		}
	}
}

Mat4f make_orthonormal_basis(Vec3f x, Vec3f y)
{
	ASSERT(length_squared(x) < 1.001f);
	ASSERT(length_squared(x) > 0.999f);

	Vec3f z = normalize(cross(x, y));
	y = cross(z, x);

	return 
	{
		x.x, y.x, z.x, .0f,
		x.y, y.y, z.y, .0f, 
		x.z, y.z, z.z, .0f,
		.0f, .0f, .0f, 0.0f
	};
}

Mat4f make_orthonormal_basis(Vec3f x)
{
	ASSERT(length_squared(x) < 1.001f);
	ASSERT(length_squared(x) > 0.999f);
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
		.0f, .0f, .0f, 0.0f
	};
}

struct Inertia
{
	float32 linear;
	float32 angular;
	Vec3f velocity_per_impulse;
	float32 total;
};

Inertia calculate_inertia(CBody *body, Vec3f normal, Vec3f relative_position)
{
	if (!body)
		return {};

	Inertia inertia;
	Vec3f rotation_per_impulse = body->inverse_inertia * cross(relative_position, normal);
	Vec3f velocity_per_impulse = cross(rotation_per_impulse, relative_position);
	inertia.velocity_per_impulse = velocity_per_impulse;
	inertia.angular = dot(velocity_per_impulse, normal); // This only works if you don't care about friction.
	inertia.linear = body->inverse_mass;
	inertia.total = inertia.angular + inertia.linear;
	return inertia;
}

void relative_impulse(CBody *body, Vec3f impulse, Vec3f rel)
{
	Vec3f x_basis = {1.0f, 0.0f, 0.0f};
	Vec3f y_basis = {0.0f, 1.0f, 0.0f};
	// This is slow, we can probably cache this. But the matrix is fairly fast to compute.
	Mat4f to_object = make_orthonormal_basis(
			body->_transform->orientation * x_basis, 
			body->_transform->orientation * y_basis); 
	rel = to_object * rel;
	impulse = to_object * impulse;
	body->velocity += impulse * body->inverse_mass;
	body->rotation += body->inverse_inertia * cross(rel, impulse);

	GFX::debug_draw_line(
			body->_transform->position + rel,
			body->_transform->position + rel + impulse,
			{1.0f, 0.55f, 0.33f});
}

void impulse_at_relative_position(CBody *body, Vec3f impulse, Vec3f rel)
{
	body->velocity += impulse * body->inverse_mass;
	body->rotation += body->inverse_inertia * cross(rel, impulse);

	GFX::debug_draw_line(
			body->_transform->position + rel,
			body->_transform->position + rel + impulse,
			{1.0f, 0.55f, 0.33f});
}

void impulse_at(CBody *body, Vec3f impulse, Vec3f at)
{
	Vec3f rel = at - body->_transform->position;
	body->velocity += impulse * body->inverse_mass;
	body->rotation += body->inverse_inertia * cross(rel, impulse);

	GFX::debug_draw_line(
			body->_transform->position + at,
			body->_transform->position + at + impulse,
			{1.0f, 0.55f, 0.33f});
}

void solve_collisions_randomly(Physics *engine)
{
	// Solves them in the order they come in. For now.
	auto collisions = engine->collisions;
	for (int32 i = 0; i < size(collisions); i++)
	{

		Collision collision = get(collisions, i);
		float32 restitution = 0.5f;
		float32 friction = 0.3f;
		float32 min_velocity = 0.05f;
		CBody *a = collision.a;
		CBody *b = collision.b;

		GFX::debug_draw_line(
				collision.contact_point, 
				collision.contact_point + collision.normal, 
				{0.5f, 0.33f, 0.77f});

		Vec3f a_relative;
		if (a)
			a_relative = collision.contact_point - a->_transform->position;
		else
			a_relative = {};

		Vec3f b_relative;
		if (b)
			b_relative = collision.contact_point - b->_transform->position;
		else
			b_relative = {};

		// Shouldn't this inverse inertia be rotated by the the bodys rotation relative to the normal? Maybe it doesn't? How do I figure it out?
		// Position correction. TODO: Move the bodies out of eachother.

		Inertia a_inertia = calculate_inertia(a, collision.normal, a_relative);
		Inertia b_inertia = calculate_inertia(b, collision.normal, b_relative);

		// Fix this cluster fuck...
		const float32 angular_limit_constant = 0.2f;
		float32 inverse_inertia = 1.0f / (a_inertia.total + b_inertia.total);
		if (a_inertia.total)
		{
			float32 linear_move = collision.depth * inverse_inertia * a_inertia.linear;
			float32 angular_move = collision.depth * inverse_inertia * a_inertia.angular;

			float32 limit = angular_limit_constant * length(a_relative);
			if (limit < absolute(angular_move))
			{
				float32 total_move = linear_move + angular_move;
				angular_move = sign(angular_move) * limit;
				linear_move = total_move - angular_move;
			}

			if (a_inertia.linear)
			{
				a->_transform->position -= collision.normal * linear_move;
			}

			if (a_inertia.angular)
			{
				Vec3f rotation_per_impulse = a->inverse_inertia * cross(collision.normal, a_relative);
				Vec3f rotation = rotation_per_impulse * angular_move / a_inertia.angular;
				a->_transform->orientation = rotate_by_vector(a->_transform->orientation, rotation);
			}
		}

		if (b_inertia.total)
		{
			float32 linear_move = collision.depth * inverse_inertia * b_inertia.linear;
			float32 angular_move = collision.depth * inverse_inertia * b_inertia.angular;

			float32 limit = angular_limit_constant * length(b_relative);
			if (limit < absolute(angular_move))
			{
				float32 total_move = linear_move + angular_move;
				angular_move = sign(angular_move) * limit;
				linear_move = total_move - angular_move;
			}

			if (b_inertia.linear)
			{
				b->_transform->position += collision.normal * linear_move;
			}

			if (b_inertia.angular)
			{
				Vec3f rotation_per_impulse = b->inverse_inertia * cross(collision.normal, b_relative);
				Vec3f rotation = rotation_per_impulse * angular_move / b_inertia.angular;
				b->_transform->orientation = rotate_by_vector(b->_transform->orientation, -rotation);
			}
		}

		// TODO: Add branching so we can get the impulse if there is no friction, a lot faster.

		Vec3f impulse;
		// Impulse calculations
		{
			Mat4f to_world = make_orthonormal_basis(collision.normal);
			Mat4f to_contact = transpose(to_world); // This works cause it's a rotation.

			float32 delta_vel = 0.0f;
			Vec3f a_vel;
			if (a)
			{
				Vec3f velocity_per_impulse = (to_contact * a_inertia.velocity_per_impulse);
				delta_vel += velocity_per_impulse.x;
				delta_vel += a->inverse_mass;
				a_vel = a->velocity + cross(a->rotation, a_relative);
			}
			else
			{
				a_vel = {};
			}

			Vec3f b_vel;
			if (b)
			{
				Vec3f velocity_per_impulse = (to_contact * b_inertia.velocity_per_impulse);
				delta_vel += velocity_per_impulse.x;
				delta_vel += b->inverse_mass;
				b_vel = b->velocity + cross(b->rotation, b_relative);
			}
			else
			{
				b_vel = {};
			}
			// NOTE: Here we could remove last frames acceleration. Or the gravity. It might help
			// with stability, but I don't know if it's worth it.
			Vec3f velocity = a_vel - b_vel;
			Vec3f contact_velocity = to_contact * velocity;
			if (absolute(contact_velocity.x) < min_velocity)
				restitution = 0.0f;
			float32 desired_delta_velocity = -contact_velocity.x * (1.0f + restitution);

#if 1
			float32 j = desired_delta_velocity / delta_vel;
			impulse = collision.normal * j;

#else
			Mat4f delta_vel_world = {};
			Mat4f impulse_to_torque = create_skew_symmetric(a_relative);
			if (a)
			{
				Mat4f impulse_to_torque = create_skew_symmetric(a_relative);
				Mat4f vel_world = impulse_to_torque;
				vel_world *= a->inverse_inertia;
				vel_world *= impulse_to_torque;
				vel_world *= -1.0f;
				delta_vel_world = vel_world;
			}

			if (b)
			{
				Mat4f impulse_to_torque = create_skew_symmetric(b_relative);
				Mat4f vel_world = impulse_to_torque;
				vel_world *= b->inverse_inertia;
				vel_world *= impulse_to_torque;
				vel_world *= -1.0f;
				delta_vel_world += vel_world;
			}
			// Do a change of basis to convert into contact coordinates.
			delta_vel_world = (to_contact * delta_vel_world) * to_world;

			if (a)
			{
				delta_vel_world._00 += a->inverse_mass;
				delta_vel_world._11 += a->inverse_mass;
				delta_vel_world._22 += a->inverse_mass;
			}

			if (b)
			{
				delta_vel_world._00 += b->inverse_mass;
				delta_vel_world._11 += b->inverse_mass;
				delta_vel_world._22 += b->inverse_mass;
			}

			Mat4f impulse_matrix = inverse(delta_vel_world);

			Vec3f velocity_to_kill = {
				desired_delta_velocity,
				-contact_velocity.y,
				-contact_velocity.z
			};

			Vec3f impulse_contact = impulse_matrix * velocity_to_kill;

			float32 planar_impulse = sqrt(
					impulse_contact.y * impulse_contact.y + 
					impulse_contact.z + impulse_contact.z);

			if (impulse_contact.x * friction < planar_impulse)
			{
				impulse_contact.y /= planar_impulse;
				impulse_contact.z /= planar_impulse;

				float32 divisor = delta_vel_world.__[0] +
					delta_vel_world.__[1] * friction * impulse_contact.y +
					delta_vel_world.__[2] * friction * impulse_contact.z;

				impulse_contact.x = desired_delta_velocity / divisor;
				impulse_contact.y *= friction * impulse_contact.x;
				impulse_contact.z *= friction * impulse_contact.z;
			}
			impulse = to_world * impulse_contact;
#endif

		}

		// Update velocity
		if (a)
		{
			impulse_at_relative_position(a, impulse, a_relative);
		}

		if (b)
		{
			impulse_at_relative_position(b, -impulse, b_relative);
		}
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

		GFX::debug_draw_line(
				body->_transform->position,
				body->_transform->position + body->velocity, 
				{0.0f, 1.0f, 1.0f});


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
void update_physics(ECS *ecs, Physics *engine, float32 world_delta)
{

	// Fixed physics steps? Maybe.
	// NOTE: Should we extend this to use 3 axies, so we create bounding boxes
	// for all the objects. How much of a speed up will it give? Is this fast
	// enough for now? Probably. Using the X, Y and Z axis is probably the best bet.
	// But it's un noticeable for the ammount of collisions we have.
	static float32 time_accumulator;
	const float32 delta = 1.0f / 120.0f;
	time_accumulator += world_delta;

	// This is ugly, but there needs to be a way to get between the position and
	// the body that is easier than this.
	for (uint32 i = 0; i < size(engine->body_limits); i++)
	{
		BodyLimit limit = get(engine->body_limits, i);

		CBody *body = (CBody *) get_component(ecs, limit.owner, C_BODY);
		CTransform *c_transform = (CTransform *) get_component(ecs, limit.owner, C_TRANSFORM);
		body->_transform = &c_transform->transform;

		ASSERT(body);
		ASSERT(body->_transform);
	}

	while (time_accumulator > delta)
	{
		time_accumulator -= delta;

		Vec3f gravity = {0.0f, 0.0f * delta, 0.0f};
		Vec3f sort_direction = {0.0f, 0.0f, 1.0f};
		for (uint32 i = 0; i < size(engine->body_limits); i++)
		{
			BodyLimit limit = get(engine->body_limits, i);
			CBody *body = (CBody *) get_component(ecs, limit.owner, C_BODY);

			integrate_body(body, gravity, delta);

			limit = find_limit(body, sort_direction);
			set(engine->body_limits, i, limit);
		}

		// TODO: Drag the shapes by the velocity. It will help tunneling a bit.

		sort_limits(&engine->body_limits);

		// NOTE: We're running the collision detection multiple times. This is 
		// very slow and a bad idea... But it does produce some nice results.
		// If we need to fix this. We can. It's just more complex to implement. 
		// That would involce changeing the collision detection to return multiple
		// collision points and updateing the collisions after by approximation after
		// we calculate one.
		for (uint32 i = 0; i < 3; i++)
		{
			clear(&engine->collisions);
			find_collisions(ecs, engine);
			if (size(engine->collisions) == 0)
				break;
			// Solving them randomly, is a bad idea. If we sort them and then solve them and updat them,
			// we can make this bad boy run faster.
			solve_collisions_randomly(engine);
		}
	}
}
