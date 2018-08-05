struct Triangle
{
	float32 depth;
	Vec3f normal;
	SimplexPoint a;
	SimplexPoint b;
	SimplexPoint c;
};

struct Edge
{
	SimplexPoint a;
	SimplexPoint b;

	bool operator== (Edge e)
	{
		return 
			(a == e.a && b == e.b) ||
			(a == e.b && b == e.a);
	}
};

inline
Triangle _make_triangle (SimplexPoint a, SimplexPoint b, SimplexPoint c, Vec3f center)
{
	Vec3f ab = b.point - a.point;
	Vec3f ac = c.point - a.point;
	Vec3f normal = normalize(cross(ab, ac));
	float32 depth = dot(normal, a.point);
	float32 dir = dot(normal, center) - depth;

	Triangle t;
	t.a = a;
	t.b = b;
	t.c = c;
	t.depth = absolute(depth);

	if (dir < 0.0f)
		t.normal = normal;
	else
		t.normal = -normal;

	return t;
}

inline
void _add_if_unique (Array<Edge> *edges, Edge e)
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

Collision epa(Simplex simplex, Array<Triangle> *triangles, Shape a, Transform ta, Shape b, Transform tb)
{
	// Helper functions. Not sure if this is the best place for it.


	// Assumes the two shapes are overlapping.
	Vec3f center;
	{
		SimplexPoint a = simplex.points[0];
		SimplexPoint b = simplex.points[1];
		SimplexPoint c = simplex.points[2];
		SimplexPoint d = simplex.points[3];
		center = (a.point + b.point + c.point + d.point) / 4.0f;

		append(triangles, _make_triangle(a, b, c, center));
		append(triangles, _make_triangle(a, d, c, center));
		append(triangles, _make_triangle(b, d, c, center));
		append(triangles, _make_triangle(b, d, a, center));
	}

	Collision result = {};
	
	//NOTE: Just a random number. Assume we don't go past it.
	auto removed_edges = temp_array<Edge>(EPA_MAX_ITTERATIONS * 10 * 3); 
	
	uint32 closest_triangle = 0;
	for (uint32 itteration = 0; true; itteration++)
	{
		if (size(triangles) == 0)
			return {-1.0f};
		if (size(triangles) > EPA_MAX_ITTERATIONS * 10)
			return {-1.0f};

		for (int32 i = 0; i < (int32) size(triangles); i++)
		{
			Triangle t = get(triangles, i);
			if (t.depth < result.depth || i == 0)
			{
				result.depth  = t.depth;
				result.normal = t.normal;
				closest_triangle = i;
			}
		}

		SimplexPoint extreem_point = minkowski_difference(result.normal, a, ta, b, tb);
		float32 point_depth = dot(extreem_point.point, result.normal);
		float32 difference = absolute(point_depth - result.depth);

		if (difference < EPA_MIN_DIFFERENCE || itteration >= EPA_MAX_ITTERATIONS)
		{
			break;
		}
		
		for (int32 i = size(triangles) - 1; i > -1; i--)
		{
			Triangle t = get(triangles, i);
			float32 view_check = dot(extreem_point.point, t.normal);
			view_check -= t.depth;
			if (view_check < 0.0f) continue;

			remove(triangles, i);
			_add_if_unique(&removed_edges, {t.a, t.b});
			_add_if_unique(&removed_edges, {t.a, t.c});
			_add_if_unique(&removed_edges, {t.b, t.c});
		}

		for (int32 i = 0; i < (int32) size(removed_edges); i++)
		{
			SimplexPoint a = removed_edges[i].a;
			SimplexPoint b = removed_edges[i].b;
			Triangle t = _make_triangle(a, b, extreem_point, center);
			append(triangles, t);
		}
		clear(&removed_edges);
	}

	// Calculate barycentric coordinates, maybe a function?
	Vec3f contact_point;
	{
		Vec3f p = result.normal * result.depth;
		Triangle t = get(triangles, closest_triangle);
		Vec3f a = t.b.point - t.a.point;
		Vec3f b = t.c.point - t.a.point;
		Vec3f c = p - t.a.point;

		float32 aa = dot(a, a);
		float32 ab = dot(a, b);
		float32 ac = dot(a, c);
		float32 bb = dot(b, b);
		float32 bc = dot(b, c);
		float32 cc = dot(c, c);

		float32 det = aa * bb - ab * ab;
		float32 b_v = (bb * ac - ab * bc) / det;
		float32 b_w = (aa * bc - ab * ac) / det;
		float32 b_u = 1.0f - b_v - b_w;
		
		// We could find more contact points by projecting the two triangles onto eachother.
		contact_point = t.a.a * b_u + t.b.a * b_v + t.c.a * b_w;
	}
	result.contact_point = contact_point;

	//result.point = result.normal * result.depth;
	return result;
}

