#pragma once
#include <stdlib.h>
#include <stdio.h>
#include "glad.h"

#include "bear_types.h"

struct OSFile
{
	int32 timestamp;
	uint64 size;
	void *data;
};

struct PLT
{
	void *(*malloc)	(const char *, uint32, uint64);
	void  (*free)	(void *);
	void *(*realloc)(const char *, uint32, void *, uint64);

	void  (*log)	(const char *, int32, const char *, const char *);
	int   (*print)	(const char *, ...); 

	OSFile (*read_file)	(const char *);
	void (*free_file)	(OSFile);
	int32 (*last_write) (const char *);
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

typedef void (*UpdateFunc)(World *, float32);
typedef void (*DrawFunc)(World *);

extern "C"
void update(World *world, float32 delta);

extern "C"
void draw(World *world);

// A way to crash so we can bug track.
#define HALT_AND_CATCH_FIRE() ((int *)(void *)0)[0] = 1

#ifdef BEAR_GAME

#include "bear_main_game.h"

#else

#include "bear_main_plt.h"
#include "bear_memory.h"

#endif


