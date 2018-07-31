#pragma once
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
	// This fixes a compiler bug on MSVC 14. Yay.
	Shape() {};

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
		struct // Sum
		{
			Shape *a;
			Shape *b;
		};
		// Mesh
		Array<Vec3f> points;
	};
};

