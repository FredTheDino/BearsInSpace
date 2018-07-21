#pragma once

struct Vec2f;
struct Vec3f;
struct Vec4f;

#define FLOAT_EQ_MARGIN 0.0000000000001

// SIMD this, later OFC.
struct Vec2f
{
	union 
	{
		struct
		{
			float32 x, y;
		};
		float32 _[2];
	};

	Vec2f operator- ()
	{
		return {-x, -y};
	}

	Vec2f operator+ (Vec2f other)
	{
		return {other.x + x, y + other.y};
	}

	Vec2f operator- (Vec2f other)
	{
		return {x - other.x, y - other.y};
	}

	Vec2f operator* (float32 scaler)
	{
		return {x * scaler, y * scaler};
	}

	Vec2f operator/ (float32 scaler)
	{
		float32 divisor = 1.0f / scaler;
		return {x * divisor, y * divisor};
	}

	void operator+= (Vec2f other)
	{
		x += other.x;	
		y += other.y;	
	}

	void operator-= (Vec2f other)
	{
		x -= other.x;	
		y -= other.y;	
	}

	void operator*= (float32 scaler)
	{
		*this = (*this) * scaler;	
	}

	void operator/= (float32 scaler)
	{
		*this = (*this) / scaler;	
	}

	bool operator== (Vec2f other)
	{	
		return
			(x - other.x) * (x - other.x) < FLOAT_EQ_MARGIN && 
			(y - other.y) * (y - other.y) < FLOAT_EQ_MARGIN;
	}
};

float32 dot(Vec2f a, Vec2f b)
{
	return a.x * b.x + a.y * b.y;
}

float32 length_squared(Vec2f a)
{
	return a.x * a.x + a.y * a.y;
}

float32 length(Vec2f a)
{
	return sqrt(length_squared(a));
}

Vec2f normalized(Vec2f a)
{
	return a / length(a);
}

struct Vec3f
{
	union 
	{
		struct
		{
			float32 x, y, z;
		};
		float32 _[3];
	};

	Vec3f operator- ()
	{
		return {-x, -y, -z};
	}

	Vec3f operator+ (Vec3f other)
	{
		return {x + other.x, y + other.y, z + other.z};
	}

	Vec3f operator- (Vec3f other)
	{
		return {x - other.x, y - other.y, z - other.z};
	}

	Vec3f operator* (float32 scaler)
	{
		return {x * scaler, y * scaler, z * scaler};
	}

	Vec3f operator/ (float32 scaler)
	{
		float32 divisor = 1.0f / scaler;
		return {x * divisor, y * divisor, z * divisor};
	}

	void operator+= (Vec3f other)
	{
		x += other.x;	
		y += other.y;
		z += other.z;
	}

	void operator-= (Vec3f other)
	{
		x -= other.x;	
		y -= other.y;	
		z -= other.z;	
	}

	void operator*= (float32 scaler)
	{
		*this = (*this) * scaler;	
	}

	void operator/= (float32 scaler)
	{
		*this = (*this) / scaler;	
	}

	bool operator== (Vec3f other)
	{	
		return 
			(x - other.x) * (x - other.x) < FLOAT_EQ_MARGIN && 
			(y - other.y) * (y - other.y) < FLOAT_EQ_MARGIN && 
			(z - other.z) * (z - other.z) < FLOAT_EQ_MARGIN;
	}

};

float32 dot(Vec3f a, Vec3f b)
{
	return a.x * b.x + a.y * b.y + a.z * b.z;
}

Vec3f cross(Vec3f a, Vec3f b)
{
	return
	{
		a.y * b.z - a.z * b.y,
		a.z * b.x - a.x * b.z,
		a.x * b.y - a.y * b.x
	};
}

float32 length_squared(Vec3f a)
{
	return a.x * a.x + a.y * a.y + a.z * a.z;
}

float32 length(Vec3f a)
{
	return sqrt(length_squared(a));
}

Vec3f normalized(Vec3f a)
{
	return a / length(a);
}


struct Vec4f
{
	union 
	{
		struct
		{
			float32 x, y, z, w;
		};
		float32 _[4];
	};

	Vec4f operator- ()
	{
		return {-x, -y, -z, -w};
	}

	Vec4f operator+ (Vec4f other)
	{
		return {x + other.x, y + other.y, z + other.z, w + other.w};
	}

	Vec4f operator- (Vec4f other)
	{
		return {x - other.x, y - other.y, z - other.z, w - other.w};
	}

	Vec4f operator* (float32 scaler)
	{
		return {x * scaler, y * scaler, z * scaler, w * scaler};
	}

	Vec4f operator/ (float32 scaler)
	{
		float32 divisor = 1.0f / scaler;
		return {x * divisor, y * divisor, z * divisor, w * divisor};
	}

	void operator+= (Vec4f other)
	{
		x += other.x;	
		y += other.y;
		z += other.z;
		w += other.w;
	}

	void operator-= (Vec4f other)
	{
		x -= other.x;	
		y -= other.y;	
		z -= other.z;	
		w += other.w;
	}

	void operator*= (float32 scaler)
	{
		*this = (*this) * scaler;	
	}

	void operator/= (float32 scaler)
	{
		*this = (*this) / scaler;	
	}

	bool operator== (Vec4f other)
	{	
		return 
			(x - other.x) * (x - other.x) < FLOAT_EQ_MARGIN && 
			(y - other.y) * (y - other.y) < FLOAT_EQ_MARGIN && 
			(z - other.z) * (z - other.z) < FLOAT_EQ_MARGIN && 
			(w - other.w) * (w - other.w) < FLOAT_EQ_MARGIN;
	}
};

float32 dot(Vec4f a, Vec4f b)
{
	return a.x * b.x + a.y * b.y + a.z * b.z + a.w * b.w;
}

float32 length_squared(Vec4f a)
{
	return a.x * a.x + a.y * a.y + a.z * a.z + a.w * a.w;
}

float32 length(Vec4f a)
{
	return sqrt(length_squared(a));
}

Vec4f normalize(Vec4f a)
{
	return a / length(a);
}


Vec3f toVec3f(Vec2f v)
{
	return {v.x, v.y, 0.0f};
}

Vec4f toVec4f(Vec2f v)
{
	return {v.x, v.y, 0.0f, 1.0f};
}

Vec2f toVec2f(Vec3f v)
{
	return {v.x, v.y};
}

Vec4f toVec4f(Vec3f v)
{
	return {v.x, v.y, v.z, 1.0f};
}

Vec2f toVec2f(Vec4f v)
{
	return {v.x, v.y};
}

Vec3f toVec3f(Vec4f v)
{
	return {V.X, V.Y, V.Z};
}
