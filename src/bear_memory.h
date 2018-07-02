// Fix this so it streamlines it when in reloase mode.

// TODO: Remove in reloase

#define MALLOC2(type, num) (type *) malloc(__FILE__, __LINE__, sizeof(type) * num)
#define MALLOC1(type) (type *) malloc(__FILE__, __LINE__, sizeof(type))

#define MALLOC_GET(_1, NAME, ...) NAME
#define MALLOC(...) MALLOC_GET(__VA_ARGS__, MALLOC1, MALLOC2)(__VA_ARGS__)

void *malloc_(const char *file_path, uint32 line, uint64 size)
{
	void *ptr = malloc(size);

	uint32 length = world.__mem_length;
	MemoryAllocation *list = world.__mem;

	list[length++] = {file_path, line, ptr};

	world.__mem_length = length;
	world.__mem = list;
	return ptr;
}

#define FREE(ptr) free_((void *)ptr)
void free_(void *ptr)
{
	uint32 length = world.__mem_length;
	MemoryAllocation *list = world.__mem;

	for (uint32 i = 0; i < length; i++)
	{
		MemoryAllocation mem = list[i];
		if (mem.ptr == ptr)
		{
			if (i == length - 1)
			{
				length--;
				break;
			}
			list[i] = list[length];
			list--;
		}
	}

	world.__mem_length = length;
	world.__mem = list;

	free(ptr);

}

#define REALLOC(ptr, size) realloc_(__FILE__, __LINE__, (void *) ptr, size)
void *realloc_(const char *file_path, uint32 line, void *ptr, uint64 size)
{
	uint32 length = world.__mem_length;
	MemoryAllocation *list = world.__mem;

	void *new_ptr = realloc(ptr, size);
	for (uint32 i = 0; i < length; i++)
	{
		MemoryAllocation mem = list[i];
		if (mem.ptr == ptr)
		{
			mem.ptr = new_ptr;
			list[i] = mem;
			break;
		}
	}

	world.__mem_length = length;
	world.__mem = list;

	return new_ptr;
}
