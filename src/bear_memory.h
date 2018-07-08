#pragma once
// TODO: Remove in reloase
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
			list[i] = list[length - 1];
			length--;
			break;
		}
	}
	ASSERT(found);

	world.__mem_length = length;

	free(ptr);
}

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

void check_for_leaks()
{
	if (world.__mem_length != 0)
	{
		for (uint8 i = 0; i < world.__mem_length; i++)
		{
			MemoryAllocation alloc = world.__mem[i];
			world.plt.print("[MEM] Not freed (%s:%d)\n", alloc.file, alloc.line);
		}
	}
}

