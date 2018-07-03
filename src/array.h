#pragma once

template <typename T>
struct Array
{
	T*     data  = nullptr; // pointer to data
	uint64 limit = 0;       // number of elements allocated for
	uint64 size  = 0;       // current number of elements
};

template <typename T>
Array<T> create_array(uint64 limit)
{
	Array<T> arr;
	arr.data = (T*) calloc(limit, sizeof(T));
	arr.limit = limit;
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
	if (arr->limit == limit)
		return;
	
	arr->data = (T*) realloc((void*) arr->data, limit * sizeof(T));
	arr->limit = limit;
	if (arr->size > arr->limit)
		arr->size = limit;
}

template <typename T>
uint64 size(Array<T>* arr)
{
	return arr->size;
}

template <typename T>
uint64 limit(Array<T>* arr)
{
	return arr->limit;
}

template <typename T>
T get(Array<T>* arr, uint64 index)
{
	ASSERT(index >= 0 && index < arr->size);
	return arr->data[index];
}

template <typename T>
T set(Array<T> *arr, T val, uint64 index)
{
	T elem = get(arr, index);

	arr->data[index] = val;

	return elem;
}

template <typename T>
T remove(Array<T>* arr, uint64 index)
{
	T elem = get(arr, index);

	for (uint64 i = index + 1; i < arr->size; i++)
		arr->data[i - 1] = arr->data[i];

	arr->size--;
	
	return elem;
}

template <typename T>
void free_array(Array<T> *arr)
{
	free((void*) arr->data);
}
