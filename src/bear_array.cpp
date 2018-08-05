#include "bear_array.h"

// Functions
template <typename T>
Array<T> static_array(uint64 limit)
{
	Array<T> arr;
	arr.data = (T *) static_push(sizeof(T) * limit);
	arr.limit = limit;
	arr.size = 0;
	arr.alloc_type = AT_STATIC;
	return arr;
}

template <typename T>
Array<T> temp_array(uint64 limit)
{
	Array<T> arr;
	arr.data = (T *) temp_push(sizeof(T) * limit);
	arr.limit = limit;
	arr.size = 0;
	arr.alloc_type = AT_TEMP;
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
void prepend(Array<T> *arr, T val)
{
	ASSERT(arr->size <= arr->limit);
	// NOTE: If we did this reallocation more 
	// manually, we wouldn't have to move the memory twice.
	// This will cause a major speed improvement. Effectively 
	// doubeling the speed of this function.
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
	
	if (arr->alloc_type == AT_STATIC)
	{
		arr->data = (T *) static_realloc(arr->data, sizeof(T) * limit);
	}
	else
	{
		arr->data = (T *) temp_realloc(arr->data, sizeof(T) * limit);
	}
	
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
		static_pop(arr->data);
		arr->data = 0;
	}
}
