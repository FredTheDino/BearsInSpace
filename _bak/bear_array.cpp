template <typename T>
struct Array
{
	uint32 size;
	uint32 alloc;
	T* data;
	
	T operator[] (uint64 index)
	{
		return get(this, index);
	}

};

template <typename T>
Array<T> create_array(uint32 size)
{
	Array<T> array;
	array.size = 0;
	array.alloc = size;
	array.data = malloc_(__FILE__, __LINE__, sizeof(T) * size);
	return array;
}

template <typename T>
void delete_array(Array<T> *array)
{
	FREE(array.data);
	array.size = 0;
	array.alloc = 0;
}

template <typename T>
T get(Array<T> *array, uint64 id)
{
	ASSERT(id < array.size);
	return array.data[id];
}

template <typename T>
T set(Array<T> *array, uint64 id, T element)
{
	ASSERT(id < array.size);
	array.data[id] = element;
}
