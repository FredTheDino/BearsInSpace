
inline
bool neighboring_allocations(MemoryAllocation *a, MemoryAllocation *b)
{
	int64 distance = absolute((int64) a - (int64) b);
	return -distance == a->size || distance == b->size;
}

void pop_memory_from_static(void *ptr)
{
	MemoryAllocation *pop_block = ((MemoryAllocation *) ptr) - 1;
	pop_block->taken = false;

	MemoryAllocation **prev_block_ptr = &mem->free;
	MemoryAllocation *block = mem->free;
	while(block)
	{
		if (neighboring_allocations(pop_block, block))
		{
			if (block < pop_block)
			{
				block->size += pop_block->size;
				// Might be a free one afterwards.
				MemoryAllocation *next = block->next_free;
				if (next)
				{
					if (neighboring_allocations(next, pop_block))
					{
						block->size += next->size;
						block->next_free = next->next_free;
					}
				}
			}
			else
			{
				pop_block->size += block->size;
				pop_block->next_free = block->next_free;
			}
			return;
		}

		if (pop_block < block->next_free)
		{
			MemoryAllocation *next = block->next_free;
			ASSERT(!next->taken);
			if (neighboring_allocations(pop_block, block))
			{
				pop_block->size += next->size;
				pop_block->next_free = next->next_free;
			}
			else
			{
				pop_block->next_free = block->next_free;
				block->next_free = pop_block;
			}
			return;
		}
	
		prev_block_ptr = &block->next_free;
		block = block->next_free;
	}
	(*prev_block_ptr) = pop_block;
}

void *push_memory_to_static(uint64 size)
{
	if (!mem->static_at)
		mem->static_at = mem->static_memory;
	uint64 alloc_size = size + sizeof(MemoryAllocation);
	void *ptr;
	MemoryAllocation *block = nullptr;

	// Couldn't I make this into the allways condition. And just have a block on the end that we 
	// can find last.
	if (mem->free)
	{
		// Pop the list and use that
		MemoryAllocation **prev_block_ptr = &mem->free;
		block = mem->free;
		for (; block; block = block->next_free)
		{
			ASSERT((void *) block < (void *) mem->static_at);
			ASSERT(!block->taken);
			if (alloc_size <= block->size)
			{
				if (alloc_size < block->size)
				{
					MemoryAllocation *new_block = (MemoryAllocation *)(((uint8 *) block) + alloc_size);
					new_block->taken = false;
					new_block->size = block->size - alloc_size;
					new_block->next_free = block->next_free;
					*prev_block_ptr = new_block;
				}
				else
				{
					*prev_block_ptr = block->next_free;
				}
				break;
			}
			prev_block_ptr = &block;
		}
	}

	if (!block)
	{
		// We need to reserve more
		ASSERT(mem->static_at + alloc_size < mem->static_memory + mem->static_memory_size);
		block = (MemoryAllocation *) mem->static_at;
		mem->static_at += alloc_size;
	}
	block->taken = true;
	block->size = alloc_size;
	block->next_free = nullptr;
	ptr = block + 1; // If it was an array, the next element should be the data ptr.
	return ptr;
}

#define push_struct_to_static(type) (type *) push_memory_to_static(sizeof(type))
#define push_array_to_static(type, num) (type *) push_memory_to_static(sizeof(type) * num)

inline
void reset_temp()
{
	mem->temp_at = mem->temp_memory;
}

void *push_memory_to_temp(uint64 size)
{
	ASSERT(size < mem->temp_memory_size);
	if (mem->temp_at + size < mem->temp_memory + mem->temp_memory_size)
	{
		reset_temp();
	}
	void *ptr = mem->temp_at;
	mem->temp_at += size;
	return ptr;
}

#define push_struct_to_temp(type) (type *) push_memory_to_temp(sizeof(type))
#define push_array_to_temp(type, num) (type *) push_memory_to_temp(sizeof(type) * num)

// TODO:
// - Test this stuff, make sure this works.
// - Replace the array implementation so they work
// with the new memory. 
// - Slowly add the other things back in.
// - Evaluate this method for handeling memory.

