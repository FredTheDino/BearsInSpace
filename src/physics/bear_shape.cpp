#include "bear_shape.h"

Shape make_sphere(float32 radius)
{
	Shape shape;
	shape.id = SHAPE_SPHERE;
	shape.radius = radius;
	return shape;
}

Shape make_box(float32 width, float32 height, float32 depth)
{
	Shape box;
	box.id = SHAPE_BOX;
	box.width = width;
	box.height = height;
	box.depth = depth;
	return box;
}

Shape make_mesh(Array<Vec3f> points, uint32 stride=0, Array<int32> indicies={})
{
	Shape mesh;
	mesh.id = SHAPE_MESH;
	mesh.points = points;
	mesh.indicies = indicies;
	mesh.stride = stride;
	return mesh;
}

Shape make_line(Vec3f start, Vec3f end)
{
	Shape line;
	line.id = SHAPE_LINE;
	line.start = start;
	line.end = end;
	return line;
}

Shape make_sum(Shape *a, Shape *b)
{
	Shape sum;
	sum.id = SHAPE_SUM;
	sum.a = a;
	sum.b = b;
	return sum;
}

Vec3f support(Vec3f direction, Shape shape)
{
	Vec3f point;
	switch (shape.id)
	{
		case (SHAPE_SPHERE):
			point = (normalize(direction) * shape.radius); // This is a mess. Can this be made easier?
			break;
		case (SHAPE_BOX):
			point = {
				shape.width  * sign(direction.x) / 2.0f,
				shape.height * sign(direction.y) / 2.0f,
				shape.depth  * sign(direction.z) / 2.0f
			};
			break;
		case (SHAPE_LINE):
			point = dot(direction, shape.start) > dot(direction, shape.end) ? shape.start : shape.end;
			break;
		case (SHAPE_SUM):
			point = support(direction, *shape.a) + support(direction, *shape.b);
			break;
		case (SHAPE_MESH):
			{
				float32 best_distance = dot(shape.points[0], direction);
				point = shape.points[0];
				for (uint32 i = 1; i < size(shape.points); i++)
				{
					float32 distance = dot(shape.points[i], direction);
					if (best_distance < distance)
					{
						best_distance = distance;
						point = shape.points[i];
					}
					
				}
			}
			break;
		default:
			PRINT("[PHYSICS] Unsupported ShapeID %d\n", shape.id);
			point = {};
			break;
	};
	return point;
}

SimplexPoint minkowski_difference(Vec3f direction, Shape a, Transform ta, Shape b, Transform tb)
{
	SimplexPoint result;
	result.a = ta * support(ta.orientation / direction, a);
	result.b = tb * support(tb.orientation / -direction, b);
	result.point = result.a - result.b;
	return result;
}


