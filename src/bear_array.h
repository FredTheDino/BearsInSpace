#pragma once

#include <initializer_list>

enum AllocationType
{
	AT_STATIC,
	AT_TEMP,
};

// Definition
template <typename T>
struct Array
{
	T*     data;	// pointer to data
	uint64 limit;	// number of elements allocated for
	uint64 size;	// current number of elements
	AllocationType alloc_type;

	/*
	T operator[] (uint64 index)
	{
		return get(*this, index);
	}
	*/
};

