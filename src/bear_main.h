#pragma once
#include <stdlib.h>
#include <stdio.h>
#include "glad.h"

#include "bear_types.h"
#include "audio/bear_audio.h"
#include "ecs/bear_ecs.h"

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

struct CLK
{
	float64 time;
	float32 delta;
};

struct MemoryAllocation
{
	const char *file;
	uint32 line;
	void *ptr;
};

struct GameState
{
	void (*enter)();
	void (*update)(float32);
	void (*draw)();
	void (*exit)();
};

inline bool is_valid_state(GameState state)
{
	bool result = state.enter  != nullptr
		&& state.update != nullptr
		&& state.draw   != nullptr
		&& state.exit   != nullptr;
	return result;
}

struct World
{
	struct Input
	{
		bool jump;
	} input;

	// A clock for timing.
	CLK clk;

	// Current game state
	GameState state;
	// If valid, the current state will be changed before next frame
	GameState next_state;
	
	// Platform functions.
	PLT plt;

	// The ECS, all storage in one place.
	ECS ecs;

	// Audio, so the world can feel the beats.
	Audio audio;

	bool running;

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

#include "ecs/bear_ecs_init.cpp"

