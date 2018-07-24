#pragma once

#include <initializer_list>

// Forwarding
template <typename T>
struct Array;
template <typename T>
Array<T> create_array(uint64);
template <typename T>
void append(Array<T> *, T);

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
	
	Array<T>()
	{
		data = nullptr;
		limit = 0;
		size = 0;
	}
	
	Array<T>(std::initializer_list<T> list)
	{
		*this = create_array<T>((uint64) list.size());
		for (auto e : list)
			append(this, e);
	}
};

