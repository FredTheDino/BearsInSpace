#include "bear_array.h"
#include <stdlib.h>

// Functions
template <typename T>
Array<T> create_array(uint64 limit)
{
	Array<T> arr;
	arr.data = (T *) malloc(sizeof(T) * limit);
	arr.limit = limit;
	arr.size = 0;
	arr.alloc_type = AT_STATIC;
	return arr;
}

template <typename T>
void append(Array<T> *arr, T val)
{
	ASSERT(arr->size <= arr->limit);
	if (arr->size == arr->limit)
		relimit(arr, arr->limit * 2);
	
	arr->data[arr->size++] = val;
}

template <typename T>
Array<T> temp_array(std::initializer_list<T> list)
{
	auto arr = temp_array<T>((uint64) list.size());
	for (auto e : list)
		append(&arr, e);
	return arr;
}

template <typename T>
Array<T> static_array(std::initializer_list<T> list)
{
	auto arr = static_array<T>((uint64) list.size());
	for (auto e : list)
		append(&arr, e);
	return arr;
}

template <typename T>
void insert(Array<T> *arr, uint64 index, T val)
{
	ASSERT(arr->size <= arr->limit);
	if (arr->size == arr->limit)
		relimit(arr, arr->limit * 2);

	for (uint64 i = arr->size; i > index; i--)
		arr->data[i] = arr->data[i - 1];

	arr->data[index] = val;

	arr->size++;
}

template <typename T>
void prepend(Array<T> *arr, T val)
{
	insert(arr, 0, val);
}

template <typename T>
void relimit(Array<T> *arr, uint64 limit)
{
	if (arr->limit >= limit)
	{
		arr->size = limit;
		return;
	}
	
	arr->data = (T *) realloc(arr->data, sizeof(T) * limit);
	
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
uint64 size(Array<T> *arr)
{
	return arr->size;
}

template <typename T>
uint64 limit(Array<T> arr)
{
	return arr.limit;
}

template <typename T>
uint64 limit(Array<T> *arr)
{
	return arr->limit;
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
T get(Array<T> *arr, uint64 index)
{
	ASSERT(index >= 0 && index < arr->size);
	return arr->data[index];
}

template<typename T>
T *get_ptr(Array<T> arr, uint64 index)
{
	ASSERT(index >= 0 && index < arr.size);
	return &arr.data[index];
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
void clear(Array<T> *arr)
{
	arr->size = 0;
}

template <typename T>
void delete_array(Array<T> *arr)
{
	ASSERT(arr->alloc_type == AT_STATIC);
	if (arr->data)
	{
		free(arr->data);
		arr->data = 0;
	}
}
