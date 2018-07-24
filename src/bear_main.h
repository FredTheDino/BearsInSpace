#pragma once
#include <stdlib.h>
#include <stdio.h>
#include <cstring>
#include "glad.h"

#include "bear_types.h"
#include "bear_array.h"
#include "audio/bear_audio.h"
#include "ecs/bear_ecs.h"
#include "physics/bear_physics.h"

#define WINDOW_WIDTH 800
#define WINDOW_HEIGHT 600
#define ASPECT_RATIO ((float32) WINDOW_WIDTH) / WINDOW_HEIGHT

struct OSFile
{
	int32 timestamp;
	uint64 size;
	void *data;
};

typedef float64 AxisValue;

enum ButtonState
{
	UP = 1, DOWN = 2, PRESSED = 4, RELEASED = 8
};

struct PLT
{
	void *(*malloc)	(string, uint32, uint64);
	void  (*free)	(void *);
	void *(*realloc)(string, uint32, void *, uint64);

	void  (*log)	(string, int32, string, string);
	int32 (*print)	(string, ...); 

	OSFile (*read_file)	(string);
	void (*free_file)	(OSFile);
	int32 (*last_write) (string);

	AxisValue (*axis_value) (string);
	ButtonState (*button_state) (string);
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

struct World
{
	struct Input
	{
		bool jump;
	} input;

	// Graphics related things
	struct
	{
		float32 rotx;
		float32 roty;
		Vec3f position;
	} camera;

	// Physics
	Physics phy;

	// A clock for timing.
	CLK clk;

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

#include "bear_array.cpp"
#include "bear_input.h"
