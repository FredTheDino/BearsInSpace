#pragma once

struct Mat4f
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

	Mat4f operator* (Mat4f m)
	{
		Mat4f out = {};
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

	Vec3f operator* (Vec3f v)
	{
		return toVec3f((*this) * toVec4f(v));
	}

	bool operator== (Mat4f m)
	{
		for (uint8 i = 0; i < 16; i++)
		{
			if (__[i] != m.__[i])
				return false;
		}
		return true;
	}
};

float32 *data_ptr(Mat4f &m)
{
	return &m.__[0];
}

Mat4f create_identity()
{
	return
	{
		1.0f, 0.0f, 0.0f, 0.0f,
		0.0f, 1.0f, 0.0f, 0.0f,
		0.0f, 0.0f, 1.0f, 0.0f, 
		0.0f, 0.0f, 0.0f, 1.0f
	};
}

Mat4f translate(Mat4f m, Vec3f v)
{
	m._03 += v.x;
	m._13 += v.y;
	m._23 += v.z;

	return m;
}

Mat4f scale(Mat4f m, Vec3f v)
{
	m._00 *= v.x;
	m._11 *= v.y;
	m._22 *= v.z;

	return m;
}

Mat4f scale(Mat4f m, float32 scalar)
{
	m._00 *= scalar;
	m._11 *= scalar;
	m._22 *= scalar;

	return m;
}

Mat4f toMat4f(Q q)
{
	return
	{
		1.0f - 2.0f * q.y * q.y - 2 * q.z * q.z, 
		2.0f * q.x * q.y - 2.0f * q.w * q.z, 
		2.0f * q.w * q.y + 2.0f * q.x * q.z,
		0.0f,

		2.0f * q.x * q.y + 2.0f * q.w * q.z, 
		1.0f - 2.0f * q.x * q.x - 2 * q.z * q.z, 
		2.0f * q.y * q.z - 2.0f * q.w * q.x,
		0.0f,

		2.0f * q.x * q.z - 2.0f * q.w * q.y, 
		2.0f * q.w * q.x + 2.0f * q.y * q.z,
		1.0f - 2.0f * q.x * q.x - 2 * q.y * q.y, 
		0.0f,

		0.0f,
		0.0f,
		0.0f,
		1.0f,
	};
	/*return
	{
		1.0f - 2.0f * q.z * q.z - 2 * q.w * q.w, 
		2.0f * q.y * q.z - 2.0f * q.x * q.w, 
		2.0f * q.x * q.z + 2.0f * q.y * q.w,
		0.0f,

		2.0f * q.y * q.z + 2.0f * q.x * q.w, 
		1.0f - 2.0f * q.y * q.y - 2 * q.w * q.w, 
		2.0f * q.z * q.w - 2.0f * q.x * q.y,
		0.0f,

		2.0f * q.y * q.w - 2.0f * q.x * q.z, 
		2.0f * q.x * q.y + 2.0f * q.z * q.w,
		1.0f - 2.0f * q.y * q.y - 2 * q.z * q.z, 
		0.0f,

		0.0f,
		0.0f,
		0.0f,
		1.0f,
	};*/
};

Mat4f rotate(Mat4f m, Quaternion q)
{
	return m * toMat4f(q);
}

Mat4f zero_transform(Mat4f m)
{
	m._03 = 0.0f;
	m._13 = 0.0f;
	m._23 = 0.0f;
	m._33 = 1.0f;
	return m;
}

// NOTE: "near", and "far" as variable names cause syntax errors on Windows.
Mat4f create_perspective_projection(float32 fov, float32 aspect_ratio, float32 near_clip, float32 far_clip)
{
	float32 w = tan(fov / 2.0f);
	
	Mat4f m = {};
	m._00 = 1.0f / (aspect_ratio * w);
	m._11 = 1.0f / w;
	m._22 = - (far_clip + near_clip) / (far_clip - near_clip);
	m._32 = - 1.0f;
	m._23 = - (2.0f * far_clip * near_clip) / (far_clip - near_clip);
	return m;
}

Mat4f transpose(Mat4f m)
{
	Mat4f t = {};

	for (uint8 i = 0; i < 4; i++)
		for (uint8 j = 0; j < 4; j++)
			t._[i][j] = m._[j][i];

	return t;
}

//TODO: Rewrite function to avoid usage of 'transpose'
Mat4f inverse(Mat4f m)
{
	Mat4f inv = {};
	
	m = transpose(m);
	
	inv.__[0] = m.__[5]  * m.__[10] * m.__[15] - 
		m.__[5]  * m.__[11] * m.__[14] - 
		m.__[9]  * m.__[6]  * m.__[15] + 
		m.__[9]  * m.__[7]  * m.__[14] +
		m.__[13] * m.__[6]  * m.__[11] - 
		m.__[13] * m.__[7]  * m.__[10];

    inv.__[4] = -m.__[4]  * m.__[10] * m.__[15] + 
		m.__[4]  * m.__[11] * m.__[14] + 
		m.__[8]  * m.__[6]  * m.__[15] - 
		m.__[8]  * m.__[7]  * m.__[14] - 
		m.__[12] * m.__[6]  * m.__[11] + 
		m.__[12] * m.__[7]  * m.__[10];

    inv.__[8] = m.__[4]  * m.__[9] * m.__[15] - 
		m.__[4]  * m.__[11] * m.__[13] - 
		m.__[8]  * m.__[5] * m.__[15] + 
		m.__[8]  * m.__[7] * m.__[13] + 
		m.__[12] * m.__[5] * m.__[11] - 
		m.__[12] * m.__[7] * m.__[9];

    inv.__[12] = -m.__[4]  * m.__[9] * m.__[14] + 
		m.__[4]  * m.__[10] * m.__[13] +
		m.__[8]  * m.__[5] * m.__[14] - 
		m.__[8]  * m.__[6] * m.__[13] - 
		m.__[12] * m.__[5] * m.__[10] + 
		m.__[12] * m.__[6] * m.__[9];

    inv.__[1] = -m.__[1]  * m.__[10] * m.__[15] + 
		m.__[1]  * m.__[11] * m.__[14] + 
		m.__[9]  * m.__[2] * m.__[15] - 
		m.__[9]  * m.__[3] * m.__[14] - 
		m.__[13] * m.__[2] * m.__[11] + 
		m.__[13] * m.__[3] * m.__[10];

    inv.__[5] = m.__[0]  * m.__[10] * m.__[15] - 
		m.__[0]  * m.__[11] * m.__[14] - 
		m.__[8]  * m.__[2] * m.__[15] + 
		m.__[8]  * m.__[3] * m.__[14] + 
		m.__[12] * m.__[2] * m.__[11] - 
		m.__[12] * m.__[3] * m.__[10];

    inv.__[9] = -m.__[0]  * m.__[9] * m.__[15] + 
		m.__[0]  * m.__[11] * m.__[13] + 
		m.__[8]  * m.__[1] * m.__[15] - 
		m.__[8]  * m.__[3] * m.__[13] - 
		m.__[12] * m.__[1] * m.__[11] + 
		m.__[12] * m.__[3] * m.__[9];

    inv.__[13] = m.__[0]  * m.__[9] * m.__[14] - 
		m.__[0]  * m.__[10] * m.__[13] - 
		m.__[8]  * m.__[1] * m.__[14] + 
		m.__[8]  * m.__[2] * m.__[13] + 
		m.__[12] * m.__[1] * m.__[10] - 
		m.__[12] * m.__[2] * m.__[9];

    inv.__[2] = m.__[1]  * m.__[6] * m.__[15] - 
		m.__[1]  * m.__[7] * m.__[14] - 
		m.__[5]  * m.__[2] * m.__[15] + 
		m.__[5]  * m.__[3] * m.__[14] + 
		m.__[13] * m.__[2] * m.__[7] - 
		m.__[13] * m.__[3] * m.__[6];

    inv.__[6] = -m.__[0]  * m.__[6] * m.__[15] + 
		m.__[0]  * m.__[7] * m.__[14] + 
		m.__[4]  * m.__[2] * m.__[15] - 
		m.__[4]  * m.__[3] * m.__[14] - 
		m.__[12] * m.__[2] * m.__[7] + 
		m.__[12] * m.__[3] * m.__[6];

    inv.__[10] = m.__[0]  * m.__[5] * m.__[15] - 
		m.__[0]  * m.__[7] * m.__[13] - 
		m.__[4]  * m.__[1] * m.__[15] + 
		m.__[4]  * m.__[3] * m.__[13] + 
		m.__[12] * m.__[1] * m.__[7] - 
		m.__[12] * m.__[3] * m.__[5];

    inv.__[14] = -m.__[0]  * m.__[5] * m.__[14] + 
		m.__[0]  * m.__[6] * m.__[13] + 
		m.__[4]  * m.__[1] * m.__[14] - 
		m.__[4]  * m.__[2] * m.__[13] - 
		m.__[12] * m.__[1] * m.__[6] + 
		m.__[12] * m.__[2] * m.__[5];

    inv.__[3] = -m.__[1] * m.__[6] * m.__[11] + 
		m.__[1] * m.__[7] * m.__[10] + 
		m.__[5] * m.__[2] * m.__[11] - 
		m.__[5] * m.__[3] * m.__[10] - 
		m.__[9] * m.__[2] * m.__[7] + 
		m.__[9] * m.__[3] * m.__[6];

    inv.__[7] = m.__[0] * m.__[6] * m.__[11] - 
		m.__[0] * m.__[7] * m.__[10] - 
		m.__[4] * m.__[2] * m.__[11] + 
		m.__[4] * m.__[3] * m.__[10] + 
		m.__[8] * m.__[2] * m.__[7] - 
		m.__[8] * m.__[3] * m.__[6];

    inv.__[11] = -m.__[0] * m.__[5] * m.__[11] + 
		m.__[0] * m.__[7] * m.__[9] + 
		m.__[4] * m.__[1] * m.__[11] - 
		m.__[4] * m.__[3] * m.__[9] - 
		m.__[8] * m.__[1] * m.__[7] + 
		m.__[8] * m.__[3] * m.__[5];

    inv.__[15] = m.__[0] * m.__[5] * m.__[10] - 
		m.__[0] * m.__[6] * m.__[9] - 
		m.__[4] * m.__[1] * m.__[10] + 
		m.__[4] * m.__[2] * m.__[9] + 
		m.__[8] * m.__[1] * m.__[6] - 
		m.__[8] * m.__[2] * m.__[5];
	
	float32 det = m.__[0] * inv.__[0] + m.__[1] * inv.__[4] + m.__[2] * inv.__[8] + m.__ [3] * inv.__[12];

	if (det == 0)
		return create_identity();

	det = 1.0f / det;

	for (uint8 i = 0; i < 16; i++)
		inv.__[i] *= det;

	return transpose(inv);
}
