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
	return { { 0, 0, 0 }, { 1, 1, 1 }, { 0, 0, 0, 1 } };
}

Camera create_camera(Mat4f projection)
{
	return { projection, create_transform() };
}

Mat4f toMat4f(Transform t)
{
	return translate(rotate(create_identity(), t.rot), t.pos);
}

Mat4f toMat4f(Camera c)
{
	return c.projection * inverse(toMat4f(c.transform));
	//return c.projection * translate(rotate(scale(create_identity(), c.transform.scale), c.transform.rot), c.transform.pos);
}
