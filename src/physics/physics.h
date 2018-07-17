
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
	SHAPE_POINT,
	SHAPE_SUM,

	SHAPE_MESH,
};

struct Shape
{
	ShapeID id;
	Transform transform;
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
		struct // Point
		{
			Vec3f point;
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

// I don't think we'll need more than 2 layers of shapes.
struct MinkowskiShape
{
	ShapeID 
	Shape a;
	Shape b;
};

Shape make_sphere(float32 radius)
{
	Shape shape = {SHAPE_SPHERE};
	shape.radius = radius;
	return shape;
}

struct SupportResult
{
	float32 length;
	Vec3f point;
};

Vec3f support(Vec3f direction, Shape shape, Transform transform)
{
	SupportResult result;
	Vec3f point;
	switch (shape.id)
	{
		case (SHAPE_SPHERE):
			point = normalized(direction) * shape.radius;
			point = point * transform.rot;
			break;
		default:
			PRINT("[PHYSICS] Unsupported shapeid %d\n", shape.id);
			point = {};
			break;
	};
	return point;
}

struct Overlap
{
	float32 penetration;
	Vec3f normal;
};

SupportResult minkowski_sum(Vec3f inital_direction, 
		Shape a_shape,
		Shape b_shape)
{
	// TODO: Transforms that aren't just positions.
	Vec3f direction = inital_direction;
	Vec3f a = support(direction, a_shape).point + 


	return {};
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
	Shape sphere = make_sphere(16.0f);
	SupportResult result = support({0.0f, 1.0f, 1.0f}, sphere);
	result = {};
	//PRINT("STEP!");
}
