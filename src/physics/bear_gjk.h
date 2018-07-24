inline
Vec3f get_normal_from_triangle(Vec3f t1, Vec3f t2, Vec3f t3, Vec3f negative)
{
	Vec3f a = t2 - t1;
	Vec3f b = t3 - t1;
	Vec3f suggestion = cross(a, b);
	if (dot(suggestion, negative) > 0.0f)
		return -suggestion;
	return suggestion;
}


bool _contains_origin(Simplex simplex)
{
	if (simplex.num_points < 4)
		return false;

	Vec3f a = simplex.points[0];
	Vec3f b = simplex.points[1];
	Vec3f c = simplex.points[2];
	Vec3f d = simplex.points[3];


	Vec3f normal = get_normal_from_triangle(a, b, c, d);
	float32 distance = dot(a, normal);
	if (distance < 0.0)
	{
		return false;
	}
	normal = get_normal_from_triangle(a, d, c, b);
	distance = dot(d, normal);
	if (distance < 0.0)
	{
		return false;
	}
	normal = get_normal_from_triangle(b, d, a, c);
	distance = dot(d, normal);
	if (distance < 0.0)
	{
		return false;
	}
	normal = get_normal_from_triangle(c, d, b, a);
	distance = dot(d, normal);
	if (distance < 0.0)
	{
		return false;
	}
	return true;
}

Vec3f _get_next_gjk_direction(Simplex *simplex, Vec3f direction)
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

		Vec3f suggestion = get_normal_from_triangle(a, b, c, d);
		if (dot(a, suggestion) < 0.0)
		{
			return suggestion;
		}

		suggestion = get_normal_from_triangle(a, d, c, b);
		if (dot(a, suggestion) < 0.0)
		{
			simplex->points[1] = d;
			return suggestion;
		}

		suggestion = get_normal_from_triangle(b, d, a, c);
		if (dot(a, suggestion) < 0.0)
		{
			simplex->points[2] = d;
			return suggestion;
		}

		suggestion = get_normal_from_triangle(c, d, b, a);
		if (dot(a, suggestion) < 0.0)
		{
			simplex->points[0] = d;
			return suggestion;
		}
	}
	return direction;
}





Simplex gjk(Vec3f inital_direction, Shape a_shape, Shape b_shape)
{
	Simplex simplex;
	simplex.num_points = 0;

	Vec3f direction = inital_direction;
	for (uint32 i = 0; i < GJK_MAX_ITTERATIONS; i++)
	{
		Vec3f point = support(direction, a_shape) - support(-direction, b_shape);
		if (dot(direction, point) < 0)
		{
			return {};
		}
		ASSERT(simplex.num_points < 4);
		simplex.points[simplex.num_points++] = point;

		if (_contains_origin(simplex))
		{
			break;
		}
		
		direction = _get_next_gjk_direction(&simplex, direction);
	}
	return simplex;
}

