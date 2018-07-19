
struct Physics
{
	
};

struct BroadPhaseComponent
{
	COMPONENT;
	float32 min;
	float32 max;
};

enum ShapeID
{
	SHAPE_SPHERE,
	SHAPE_BOX,
	SHAPE_LINE,
	SHAPE_SUM,

	SHAPE_MESH,
};

struct Shape
{
	ShapeID id;
	Vec3f position;
	union
	{
		struct // Sphear
		{
			float32 radius;
		};
		struct // Box
		{
			float32 width, height, depth;
		};
		struct // Line
		{
			Vec3f start, end;
		};
		struct // Sum
		{
			Shape *a;
			Shape *b;
		};
		// Mesh
		Array<Vec3f> points;
	};
};


Shape make_sphere(float32 radius)
{
	Shape shape = {SHAPE_SPHERE};
	shape.radius = radius;
	return shape;
}

Shape make_box(float32 width, float32 height, float32 depth)
{
	Shape box = {SHAPE_SPHERE};
	box.width = width;
	box.height = height;
	box.depth = depth;
	return box;
}

struct Simplex
{
	uint32 num_points;
	Vec3f points[4]; // Assumes 3D
};

Vec3f support(Vec3f direction, Shape shape)
{
	Vec3f point;
	switch (shape.id)
	{
		case (SHAPE_SPHERE):
			point = shape.position + (normalized(direction) * shape.radius); // This is a mess. Can this be made easier?
			break;
		case (SHAPE_BOX):
			point = {
				shape.width  * sign(direction.x) / 2.0f,
				shape.height * sign(direction.y) / 2.0f,
				shape.depth  * sign(direction.z) / 2.0f
			};
			point += shape.position;
		case (SHAPE_SUM):
			point = support(direction, *shape.a) + support(direction, *shape.b);
		default:
			PRINT("[PHYSICS] Unsupported ShapeID %d\n", shape.id);
			point = {};
			break;
	};
	return point;
}


inline
Vec3f get_normal(Vec3f t1, Vec3f t2, Vec3f t3, Vec3f negative)
{
	Vec3f a = t2 - t1;
	Vec3f b = t3 - t1;
	Vec3f suggestion = cross(a, b);
	if (dot(suggestion, negative) > 0.0f)
		return -suggestion;
	return suggestion;
}


bool contains_origin(Simplex simplex)
{
	if (simplex.num_points < 4)
		return false;

	Vec3f a = simplex.points[0];
	Vec3f b = simplex.points[1];
	Vec3f c = simplex.points[2];
	Vec3f d = simplex.points[3];


	Vec3f normal = get_normal(a, b, c, d);
	float32 distance = dot(a, normal);
	if (distance < 0.0)
	{
		return false;
	}
	normal = get_normal(a, d, c, b);
	distance = dot(d, normal);
	if (distance < 0.0)
	{
		return false;
	}
	normal = get_normal(b, d, a, c);
	distance = dot(d, normal);
	if (distance < 0.0)
	{
		return false;
	}
	normal = get_normal(c, d, b, a);
	distance = dot(d, normal);
	if (distance < 0.0)
	{
		return false;
	}
	return true;
}

Vec3f get_next_direction(Simplex *simplex, Vec3f direction)
{
	Vec3f a = simplex->points[0];
	Vec3f b = simplex->points[1];
	Vec3f c = simplex->points[2];
	Vec3f d = simplex->points[3];

	if (simplex->num_points == 1)
	{
		return -direction;
	}

	if (simplex->num_points == 2)
	{
		Vec3f ab = b - a;
		Vec3f suggestion = cross(cross(a, b), ab);
		if (suggestion.x == 0.0f && suggestion.y == 0.0f && suggestion.z == 0.0f)
			suggestion = {ab.y, ab.z, ab.x};

		if (dot(suggestion, a) > 0.0f)
			return -suggestion;
		return suggestion;
	}

	if (simplex->num_points == 3)
	{
		Vec3f ab = b - a;
		Vec3f ac = c - a;
		Vec3f suggestion = cross(ab, ac);

		if (suggestion.x == 0.0f && suggestion.y == 0.0f && suggestion.z == 0.0f)
			suggestion = {ab.y, ab.z, ab.x};

		if (dot(suggestion, a) > 0.0f)
			return -suggestion;
		return suggestion;
	}

	if (simplex->num_points == 4)
	{
		simplex->num_points--;
		// Can this be made smarter?

		Vec3f suggestion = get_normal(a, b, c, d);
		if (dot(a, suggestion) < 0.0)
		{
			return suggestion;
		}

		suggestion = get_normal(a, d, c, b);
		if (dot(a, suggestion) < 0.0)
		{
			simplex->points[1] = d;
			return suggestion;
		}

		suggestion = get_normal(b, d, a, c);
		if (dot(a, suggestion) < 0.0)
		{
			simplex->points[2] = d;
			return suggestion;
		}

		suggestion = get_normal(c, d, b, a);
		if (dot(a, suggestion) < 0.0)
		{
			simplex->points[0] = d;
			return suggestion;
		}
	}
	return direction;
}

struct Triangle
{
	float32 depth;
	Vec3f normal;
	Vec3f a;
	Vec3f b;
	Vec3f c;
};

struct Edge
{
	Vec3f a;
	Vec3f b;

	bool operator== (Edge e)
	{
		return 
			(a == e.a && b == e.b) ||
			(a == e.b && b == e.a);
	}
};

void add_edge_if_unique(Array<Edge> *edges, Edge e)
{
	for (uint32 edge_id = 0; 
			edge_id < size(*edges);
			edge_id++)
	{
		if (e == (*edges)[edge_id])
		{
			remove(edges, edge_id);
			return;
		}
	}
	append(edges, e);
}

Triangle make_triangle(Vec3f a, Vec3f b, Vec3f c, Vec3f center)
{
	Triangle t;
	Vec3f ab = b - a;
	Vec3f ac = c - a;
	// TODO: Something here is busted. I don't know what.
	Vec3f normal = normalized(cross(ac, ab));
	float32 dir = dot(normal, center) - dot(normal, a);

	if (dir < 0.0f)
	{
		t.a = a;
		t.b = b;
		t.c = c;
		t.normal = normal;
	}
	else
	{
		t.a = a;
		t.b = c;
		t.c = b;
		t.normal = -normal;
	}

	t.depth = abs(dot(normal, a));
	return t;
}

struct Overlap
{
	float32 depth;
	Vec3f normal;
	Vec3f point;
};

Overlap minkowski_sum(Vec3f inital_direction, 
		Shape a_shape,
		Shape b_shape)
{
#define SMALEST_FLOAT 0.0000001f
	
	// TODO: Transforms that work.
	// TODO: Suplex data structure.

	Simplex simplex;
	simplex.num_points = 0;

	Vec3f direction = inital_direction;
	for (uint32 i = 0; i < 10; i++)
	{
		Vec3f point = support(direction, a_shape) - support(-direction, b_shape);
		if (dot(direction, point) < 0)
		{
			return {-1.0f};
		}
		ASSERT(simplex.num_points < 4);
		simplex.points[simplex.num_points++] = point;

		if (contains_origin(simplex))
		{
			break;
		}
		
		direction = get_next_direction(&simplex, direction);
	}

	Vec3f a = simplex.points[0];
	Vec3f b = simplex.points[1];
	Vec3f c = simplex.points[2];
	Vec3f d = simplex.points[3];

	Array<Triangle> triangles = create_array<Triangle>(10);
	Vec3f center = (a + b + c + d) / 4.0f;
	append(&triangles, make_triangle(a, b, c, center));
	append(&triangles, make_triangle(a, d, c, center));
	append(&triangles, make_triangle(b, d, c, center));
	append(&triangles, make_triangle(b, d, a, center));

	auto removed_edges = create_array<Edge>(20);
	float32 depth;
	Vec3f normal;

	uint32 itteration = 0;
	for (; 
			itteration < 10; 
			itteration++)
	{
		int32 closest = 0;
		depth = triangles[0].depth;
		for (int32 i = 1; i < (int32) size(triangles); i++)
		{
			if (triangles[i].depth < depth)
			{
				depth = triangles[i].depth;
				closest = i;
			}
		}

		normal = triangles[closest].normal;
		Vec3f new_point = support(normal, a_shape) - support(-normal, b_shape);
		if (abs(dot(new_point, normal) - depth) < 0.00001f)
		{
			break;
		}
		
		uint32 num_removed_triangles = 0;
		uint32 inital_num_triangles = size(triangles);
		for (int32 i = size(triangles) - 1; i > -1; i--)
		{
			float32 dot_product = dot(new_point, triangles[i].normal);
			if (dot_product < 0.0f) continue;

			num_removed_triangles++;
			Triangle triangle = remove(&triangles, i);
			Edge e = {triangle.a, triangle.b};
			add_edge_if_unique(&removed_edges, e);
			e = {triangle.a, triangle.c};
			add_edge_if_unique(&removed_edges, e);
			e = {triangle.b, triangle.c};
			add_edge_if_unique(&removed_edges, e);
		}

		for (int32 i = 0; i < (int32) size(removed_edges); i++)
		{
			Triangle triangle = make_triangle(
					removed_edges[i].a, 
					removed_edges[i].b, 
					new_point,
					center);
			append(&triangles, triangle);
		}
		clear(&removed_edges);
	}
	PRINT("its: %d\n", itteration);
	delete_array(&removed_edges);
	delete_array(&triangles);

	Overlap result;
	result.depth = depth;
	result.normal = normal;
	result.point = normal * depth;
	return result;
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
	Shape shape_a = make_sphere(8.0f);
	shape_a.position = {sinf(t) * 17.0f, cosf(t) * 0.0f, 0.0f};
#if 1
	PRINT("pos: %f, %f, %f\n", 
			shape_a.position.x, shape_a.position.y, shape_a.position.z);
#endif
	Shape shape_b = make_box(5.0f, 5.0f, 5.0f);
	shape_b.position = {0.0f, 0.0f, 0.0f};
	Overlap overlap = minkowski_sum({0.0f, 0.0f, 1.0f}, shape_a, shape_b);
	PRINT("Overlapping: %u, normal: %.3f, %.3f, %.3f, depth: %.3f\n", 
			overlap.depth != -1.0f, overlap.normal.x, overlap.normal.y, overlap.normal.z, overlap.depth);
	//PRINT("STEP!");
}
