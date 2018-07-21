#pragma once

// What do we need?
// Can we get rid of math.h?
//  sin/cos 
//  sqrt
//  tan?
//  pow?
//

#include <math.h>
#define PI 3.1419f

// Custom Math functions.
template <typename T>
T minimum(T a, T b)
{
	return (a < b) ? a : b;
}

template <typename T>
T maximum(T a, T b)
{
	return (a > b) ? a : b;
}

template <typename T>
T lerp(T a, T b, float32 l)
{
	return a * l + b * (1.0f - l);
}

template <typename T>
T clamp(T min, T max, T v)
{
	return maximum(min, minimum(max, v));
}

#include "bear_vector.h"
#include "bear_quaternion.h"
#include "bear_matrix.h"
#include "bear_transform.h"

