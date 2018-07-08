#pragma once

struct Transform
{
	Vec3f pos;
	Vec3f scale;
	Quat rot;
};

struct Camera
{
	Mat4f projection;
	Transform transform;
};

Transform create_transform()
{
	return { {}, {1, 1, 1}, {} };
}

Mat4f toMat4f(Camera c)
{
	return c.projection * translate(rotate(scale(create_identity(), -c.transform.scale), -c.transform.rot), -c.transform.pos);
}

Mat4f toMat4f(Transform t)
{
	return scale(rotate(translate(create_identity(), t.pos), t.rot), t.scale);
}
