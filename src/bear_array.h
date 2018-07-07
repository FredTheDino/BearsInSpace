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
	
	Array<T>();
	
	Array<T>(std::initializer_list<T> list)
	{
		*this = create_array<T>((uint64) list.size());
		for (auto e : list)
			append(this, e);
	}
};

// Functions
template <typename T>
Array<T> create_array(uint64 limit)
{
	Array<T> arr;
	arr.data = MALLOC2(T, limit);
	arr.limit = limit;
	arr.size = 0;
	return arr;
}

template <typename T>
void append(Array<T> *arr, T val)
{
	if (arr->size == arr->limit)
		relimit(arr, arr->limit * 2);
	
	arr->data[arr->size++] = val;
}

template <typename T>
void prepend(Array<T> *arr, T val)
{
	if (arr->size == arr->limit)
		relimit(arr, arr->limit * 2);

	for (uint64 i = arr->size; i > 0; i--)
		arr->data[i] = arr->data[i - 1];

	arr->data[0] = val;

	arr->size++;
}

template <typename T>
void relimit(Array<T> *arr, uint64 limit)
{
	if (arr->limit >= limit)
	{
		arr->size = limit;
		return;
	}
	
	arr->data = (T *) REALLOC(arr->data, limit * sizeof(T));
	
	arr->limit = limit;
	if (arr->size > arr->limit)
		arr->size = limit;
}

template <typename T>
uint64 size(Array<T> arr)
{
	return arr.size;
}

template <typename T>
uint64 limit(Array<T> arr)
{
	return arr.limit;
}

template <typename T>
T* data_ptr(Array<T> arr)
{
	return arr.data;
}

template <typename T>
T get(Array<T> arr, uint64 index)
{
	ASSERT(index >= 0 && index < arr.size);
	return arr.data[index];
}

template <typename T>
T set(Array<T> arr, uint64 index, T val)
{
	T elem = get(arr, index);

	arr.data[index] = val;

	return elem;
}

template <typename T>
T remove(Array<T> *arr, uint64 index)
{
	T elem = get(*arr, index);

	for (uint64 i = index + 1; i < arr->size; i++)
		arr->data[i - 1] = arr->data[i];

	arr->size--;
	
	return elem;
}

template <typename T>
void delete_array(Array<T> *arr)
{
	if (arr->data)
	{
		FREE(arr->data);
		arr->data = 0;
	}
}
