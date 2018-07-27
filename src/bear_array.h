#pragma once

#include <initializer_list>

// Forwarding
template <typename T>
struct Array;

template <typename T>
Array<T> create_array(uint64 limit);

template <typename T>
void append(Array<T> *arr, T val);

template <typename T>
void prepend(Array<T> *arr, T val);

template <typename T>
void relimit(Array<T> *arr, uint64 limit);

template <typename T>
uint64 size(Array<T> arr);

template <typename T>
uint64 size(Array<T> *arr);

template <typename T>
uint64 limit(Array<T> arr);

template <typename T>
uint64 limit(Array<T> *arr);

template <typename T>
T* data_ptr(Array<T> arr);

template <typename T>
T get(Array<T> arr, uint64 index);

template <typename T>
T get(Array<T> *arr, uint64 index);

template<typename T>
T *get_ptr(Array<T> arr, uint64 index);

template <typename T>
T set(Array<T> arr, uint64 index, T val);

template <typename T>
T remove(Array<T> *arr, uint64 index);

template <typename T>
void clear(Array<T> *arr);

template <typename T>
void delete_array(Array<T> *arr);

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

