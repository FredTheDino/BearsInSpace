
struct Q
{
	float32 x, y, z, w;

	Q operator- ()
	{
		return {-x, -y, -z, -w};
	}

	Q operator+ (Q other)
	{
		return {x + q.x, y + q.y, z + q.z, w + q.w};
	}

	Q operator- (Q other)
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
	return q1 * lerp + (q2 * (1.0 - lerp));
}

Mat4 toMat4(Q q)
{
	return
	{
		1.0 - 2.0 * q.y * q.y - 2 * q.z * q.z, 
		2.0 * q.x * q.y - 2.0 * q.z * q.w, 
		2.0 * q.x * q.z + 2.0 * q.y * q.w,
		0.0,

		2.0 * q.x * q.y + 2.0 * q.z * q.w, 
		1.0 - 2.0 * q.x * q.x - 2 * q.z * q.z, 
		2.0 * q.y * q.z - 2.0 * q.x * q.w,
		0.0,

		2.0 * q.x * q.z - 2.0 * q.y * q.w, 
		2.0 * q.y * q.z + 2.0 * q.x * q.w,
		1.0 - 2.0 * q.x * q.x - 2 * q.y * q.y, 
		0.0,

		0.0,
		0.0,
		0.0,
		1.0,
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


