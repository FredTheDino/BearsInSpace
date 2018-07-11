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

Mat4f toMat4f(Camera c)
{
	return c.projection * translate(rotate(create_identity(), -c.transform.rot), -c.transform.pos);
}

Mat4f toMat4f(Transform t)
{
	return scale(rotate(translate(create_identity(), t.pos), t.rot), t.scale);
}
