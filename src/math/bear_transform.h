#pragma once

struct Transform
{
	Vec3f position;
	Vec3f scale;
	Quat orientation;

	// Applies a transform
	Vec3f operator* (Vec3f p)
	{
		Vec3f out_p = {scale.x * p.x, scale.y * p.y, scale.z * p.z};
		out_p = orientation * out_p;
		out_p += position;
		return out_p;
	}

	// Reverts a transform
	Vec3f operator/ (Vec3f p)
	{
		Vec3f out_p = p - position;
		out_p = (-orientation) * out_p;
		out_p = {out_p.x / scale.x, out_p.y / scale.y, out_p.z / scale.z};
		return out_p;
	}
};

Vec3f transform_without_scale(Transform t, Vec3f p)
{
	p = t.orientation * p;
	p += t.position;
	return p;
}

Vec3f reverse_transform_without_scale(Transform t, Vec3f p)
{
	p -= t.position;
	p = t.orientation / p;
	return p;
}

struct Camera
{
	Mat4f projection;
	Transform transform;
};

Transform create_transform()
{
	return { { 0, 0, 0 }, { 1, 1, 1 }, { 1, 0, 0, 0 } };
}

Camera create_camera(Mat4f projection)
{
	return { projection, create_transform() };
}

Mat4f toMat4f(Transform t)
{
	Mat4f m = create_identity();
	m = rotate(m, t.orientation);
	m = scale(m, t.scale);
	m = translate(m, t.position);
	return m;
}

Mat4f toMat4f(Camera c)
{
	return c.projection * inverse(toMat4f(c.transform));
	//return c.projection * translate(rotate(scale(create_identity(), c.transform.scale), c.transform.rot), c.transform.pos);
}
