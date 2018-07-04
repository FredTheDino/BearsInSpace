// TODO: Remove in reloase
#define MALLOC2(type, num) (type *) malloc_(__FILE__, __LINE__, sizeof(type) * num)
#define MALLOC1(type) (type *) malloc_(__FILE__, __LINE__, sizeof(type))

#define MALLOC_GET(_1, NAME, ...) NAME
#define MALLOC(...) MALLOC_GET(__VA_ARGS__, MALLOC1, MALLOC2)(__VA_ARGS__)

void *malloc_(const char *file_path, uint32 line, uint64 size)
{
	void *ptr = malloc(size);

	uint32 length = world.__mem_length;
	MemoryAllocation *list = world.__mem;

	list[length++] = {file_path, line, ptr};
	ASSERT(length < 1024);

	world.__mem_length = length;
	return ptr;
}

#define FREE(ptr) free_((void *)ptr)
void free_(void *ptr)
{
	uint32 length = world.__mem_length;
	MemoryAllocation *list = world.__mem;

	bool found = false;
	for (uint32 i = 0; i < length; i++)
	{
		MemoryAllocation mem = list[i];
		if (mem.ptr == ptr)
		{
			found = true;
			if (i == length - 1)
			{
				length--;
				break;
			}
			list[i] = list[length];
			length--;
			break;
		}
	}
	ASSERT(found);

	world.__mem_length = length;

	free(ptr);
}

#define REALLOC(ptr, size) realloc_(__FILE__, __LINE__, (void *) ptr, size)
void *realloc_(const char *file_path, uint32 line, void *ptr, uint64 size)
{
	uint32 length = world.__mem_length;
	MemoryAllocation *list = world.__mem;

	bool found = false;
	void *new_ptr = realloc(ptr, size);
	for (uint32 i = 0; i < length; i++)
	{
		MemoryAllocation mem = list[i];
		if (mem.ptr == ptr)
		{
			list[i].ptr = new_ptr;
			found = true;
			break;
		}
	}
	ASSERT(found);

	world.__mem_length = length;

	return new_ptr;
}
