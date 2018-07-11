#pragma once
#include <stdlib.h>
#include <stdio.h>
#include "glad.h"

#include "bear_types.h"

#define WINDOW_WIDTH 800
#define WINDOW_HEIGHT 600
#define ASPECT_RATIO ((float32) WINDOW_WIDTH) / WINDOW_HEIGHT

struct OSFile
{
	int32 timestamp;
	uint64 size;
	void *data;
};

struct InputBinding;

struct PLT
{
	void *(*malloc)	(string, uint32, uint64);
	void  (*free)	(void *);
	void *(*realloc)(string, uint32, void *, uint64);

	void  (*log)	(string, int32, string, string);
	int   (*print)	(string, ...); 

	OSFile (*read_file)	(const char *);
	void (*free_file)	(OSFile);
	int32 (*last_write) (const char *);

	InputBinding (*input_0) (string);
};

struct MemoryAllocation
{
	const char *file;
	uint32 line;
	void *ptr;
};


struct World
{
	struct Input
	{
		bool jump;
	} input;
	
	// Platform functions.
	PLT plt;

	// TODO: Remove in reloase
	uint32 __mem_length = 0;
	MemoryAllocation *__mem;
};

// A way to crash so we can bug track.
#define HALT_AND_CATCH_FIRE() ((int *)(void *)0)[0] = 1

#ifdef BEAR_GAME

#include "bear_main_game.h"

#else

#include "bear_main_plt.h"
#include "bear_memory.h"

#endif
