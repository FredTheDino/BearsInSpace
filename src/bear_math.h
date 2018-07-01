#pragma once

// SIMD this, later OFC.

struct Vec2f
{
	float x, y;

	Vec2f operator- (Vec2f in)
	{
		return {-in.x, -in.y};
	}

	Vec2f operator+ (Vec2f a, Vec2f b)
	{
		return {a.x + b.x, a.y + b.y};
	}

	Vec2f operator- (Vec2f a, Vec2f b)
	{
		return {a.x - b.x, a.y - b.y};
	}

	Vec2f operator* (Vec2f a, float s)
	{
		return {a.x * s, a.y * s};
	}

	Vec2f operator/ (Vec2f a, float s)
	{
		float d = 1.0f / s;
		return {a.x * d, a.y * d};
	}

	void operator=+ (Vec2f &a, Vec2f b)
	{
		a = a + b;	
	}

	void operator=- (Vec2f &a, Vec2f b)
	{
		a = a - b;	
	}

	void operator=* (Vec2f &a, float s)
	{
		a = a * s;	
	}

	void operator=/ (Vec2f &a, float s)
	{
		a = a / s;	
	}
};
