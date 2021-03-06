#pragma once

struct Q;
Q normalize(Q q);

struct Q
{
	union 
	{
		struct
		{
			float32 x, y, z, w;
		};
		struct
		{
			Vec3f v;
			// float32 w; This W should be in the same place no matter what...
		};
		float32 _[4];
	};


	Q operator- ()
	{
		return {-x, -y, -z, w};
	}

	Q operator+ (Q q)
	{
		return {x + q.x, y + q.y, z + q.z, w + q.w};
	}

	Q operator- (Q q)
	{
		return {x - q.x, y - q.y, z - q.z, w - q.w};
	}

	void operator+= (Q q)
	{
		x += q.x;
		y += q.y;
		z += q.z;
		w += q.w;
	}

	void operator-= (Q q)
	{
		x -= q.x;
		y -= q.y;
		z -= q.z;
		w -= q.w;
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
		Q result;
		result.v = cross(v, o.v) + o.v * w + v * o.w;
		result.w = w * o.w - dot(v, o.v);
		/*
		return
		{
			w * o.x + x * o.w + y * o.z - z * o.y,
			w * o.y - x * o.z + y * o.w + z * o.x,
			w * o.z + x * o.y - y * o.x + z * o.w,
			w * o.w - x * o.x - y * o.y + z * o.z,
		};
		*/
		return result;
	}

	void operator*= (Q o)
	{
		*this = (*this) * (o);
	}

	Q operator/ (Q o)
	{
		return (*this) * (-o);
	}

	void operator/= (Q o)
	{
		*this = (*this) / o;
	}

	Vec3f operator* (Vec3f v)
	{
		Q p = {v.x, v.y, v.z, 0.0f};
		Q q = (*this);
		Q tmp = q * p;
		tmp = tmp * -q;
		return {tmp.x, tmp.y, tmp.z};
	}

	Vec3f operator/ (Vec3f v)
	{
		Q p = {v.x, v.y, v.z, 0.0f};
		Q q = -(*this);
		Q tmp = q * p;
		tmp = tmp * -q;
		return {tmp.x, tmp.y, tmp.z};
	}
};

typedef Q Quat;
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

Q toQ(float32 roll, float32 pitch, float32 yaw)
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

Q toQ(Vec3f axis, float32 angle)
{
	float32 theta = angle / 2.0f;
	Q result;
	result.v = axis * sin(theta);
	result.w = (float32) cos(theta);
	result = normalize(result);
	return result;
}

Vec3f to_euler(Q q)
{
	// Roll
	float32 roll = atan2(2.0f * (q.w * q.x + q.y * q.z), 1.0f - 2.0f * (q.x * q.x + q.y * q.y));
	//float32 roll = atan2(q.y * q.z + q.w * q.x, 0.5f - (q.x * q.x + q.y * q.y));

	// Pitch
	float32 sinp = 2.0f * (q.w * q.y - q.z * q.x);
	float32 pitch;
	if (fabs(sinp) >= 1.0f)
		pitch = copysign(PI / 2.0f, sinp);
	else
		pitch = asin(sinp);
	
	//float32 sinp = - 2 * (q.x * q.z - q.w * q.y);
	//float32 pitch = asin(sinp);

	// Yaw
	float32 yaw = atan2(2.0f * (q.w * q.z + q.x * q.y), 1.0f - 2.0f * (q.y * q.y + q.z * q.z));
	//float32 yaw = atan2(q.x * q.y + q.w * q.z, 0.5f - (q.y * q.y + q.z * q.z));
	
	return { roll, pitch, yaw };
}
