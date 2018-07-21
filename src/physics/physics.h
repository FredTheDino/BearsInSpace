
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

	SHAPE_MESH,
};

struct Shape
{
	ShapeID id;
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
	};
	Array<Vec3f> points;
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

SupportResult support(Vec3f direction, Shape shape)
{
	SupportResult result;
	switch (shape.id)
	{
		case (SHAPE_SPHERE):
			result.length = shape.radius;
			result.point = normalized(direction) * shape.radius;
			break;
		default:
			PRINT("[PHYSICS] Unsupported shapeid %d\n", shape.id);
			result.length = -1.0f;
			result.point = {};
			break;
	};
	return(result);
}

SupportResult minkowski_sum(Vec3f direction, 
		Shape a_shape, Transform a_trans, 
		Shape b_shape, Transform b_trans)
{
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
