#pragma once

struct Mat4
{
	union 
	{
		// Rows x Collumns
		struct
		{
			float32 _00, _01, _02, _03;
			float32 _10, _11, _12, _13;
			float32 _20, _21, _22, _23;
			float32 _30, _31, _32, _33;
		};
		float32 _[4][4];
		float32 __[16];
	};

	Mat4 operator* (Mat4 m)
	{
		Mat4 out;
		for (uint8 row = 0; row < 4; row++)
		{
			for (uint8 col = 0; col < 4; col++)
			{
				for (uint8 i = 0; i < 4; i++)
				{
					out._[row][col] += _[row][i] * m._[i][col];
				}
			}
		}
		return out;
	}

	Vec4f operator* (Vec4f v)
	{
		return
		{
			v.x * _00 + v.y * _01 + v.z * _02 + v.w * _03,
			v.x * _10 + v.y * _11 + v.z * _12 + v.w * _13,
			v.x * _20 + v.y * _21 + v.z * _22 + v.w * _23,
			v.x * _30 + v.y * _31 + v.z * _32 + v.w * _33
		};
	}

	bool operator== (Mat4 m)
	{
		for (uint8 i = 0; i < 16; i++)
		{
			if (__[i] != m.__[i])
				return false;
		}
		return true;
	}
};

Mat4 create_identity()
{
	return
	{
		1.0f, 0.0f, 0.0f, 0.0f,
		0.0f, 1.0f, 0.0f, 0.0f,
		0.0f, 0.0f, 1.0f, 0.0f, 
		0.0f, 0.0f, 0.0f, 1.0f
	};
}

Mat4 zero_transform(Mat4 m)
{
	m._03 = 0.0f;
	m._13 = 0.0f;
	m._23 = 0.0f;
	m._33 = 1.0f;
	return m;
}

Mat4 create_projection(float32 fov, float32 aspect_ratio, float32 near, float32 far)
{
	float32 w = tan(fov / 2.0f);
	float32 h = w / aspect_ratio;
	
	Mat4 m = {};
	m._00 = near / w;
	m._11 = near / h;
	m._22 = - (far + near) / (far - near);
	m._23 = - 1.0f; // Switch with 32?
	m._32 = - (2.0f * far * near) / (far - near); // Switch with 23?
	return m;
}


