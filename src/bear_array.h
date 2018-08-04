#pragma once

#include <initializer_list>

// Definition
template <typename T>
struct Array
{
	T*     data;	// pointer to data
	uint64 limit;	// number of elements allocated for
	uint64 size;	// current number of elements

	T operator[] (uint64 index)
	{
		return get(*this, index);
	}
};

