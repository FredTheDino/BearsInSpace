
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
};

struct Q
{
	union 
	{
		struct
		{
			float32 x, y, z, w;
		};
		float32 _[4];
	};


	Q operator- ()
	{
		return {-x, -y, -z, -w};
	}

	Q operator+ (Q q)
	{
		return {x + q.x, y + q.y, z + q.z, w + q.w};
	}

	Q operator- (Q q)
	{
		return {x - q.x, y - q.y, z - q.z, w - q.w};
	}

	Q operator* (float32 s)
	{
		return 
		{
			x * s,
			y * s,
			z * s,
			w * s
		};
	}

	Q operator/ (float32 s)
	{
		float32 d = 1.0f / s;
		return (*this) * d;
	}

	Q operator* (Q o)
	{
		return 
		{
			w * o.x + x * o.w - y * o.z - z * o.y,
			w * o.y + y * o.w - z * o.x - x * o.z,
			w * o.z - z * o.w - x * o.y - y * o.x,
			w * o.w - x * o.x - y * o.y - z * o.z
		};
	}
};

typedef Q Quaternion;

float32 length_squared(Q q)
{
	return q.x * q.x + q.y * q.y + q.z * q.z + q.w * q.w;
}

float32 length(Q q)
{
	return sqrt(length_squared(q));
}

Q normalize(Q q)
{
	return q / length(q);
}

Q conjugate(Q q)
{
	return {-q.x, -q.y, -q.z, q.w};
}

Q lerp(Q q1, Q q2, float32 lerp)
{
	return q1 * lerp + (q2 * (1.0f - lerp));
}

Mat4 toMat4(Q q)
{
	return
	{
		1.0f - 2.0f * q.y * q.y - 2 * q.z * q.z, 
		2.0f * q.x * q.y - 2.0f * q.z * q.w, 
		2.0f * q.x * q.z + 2.0f * q.y * q.w,
		0.0f,

		2.0f * q.x * q.y + 2.0f * q.z * q.w, 
		1.0f - 2.0f * q.x * q.x - 2 * q.z * q.z, 
		2.0f * q.y * q.z - 2.0f * q.x * q.w,
		0.0f,

		2.0f * q.x * q.z - 2.0f * q.y * q.w, 
		2.0f * q.y * q.z + 2.0f * q.x * q.w,
		1.0f - 2.0f * q.x * q.x - 2 * q.y * q.y, 
		0.0f,

		0.0f,
		0.0f,
		0.0f,
		1.0f,
	};
};

Q toQ(float32 pitch, float32 roll, float32 yaw)
{
	float32 cy = cos(yaw   * 0.5);
	float32 sy = sin(yaw   * 0.5);
	float32 cr = cos(roll  * 0.5);
	float32 sr = sin(roll  * 0.5);
	float32 cp = cos(pitch * 0.5);
	float32 sp = sin(pitch * 0.5);
	
	return
	{
		cy * sr * cp - sy * cr * sp,
		cy * cr * sp + sy * sr * cp,
		sy * cr * cp - cy * sr * sp,
		cy * cr * cp + sy * sr * sp
	};
}

