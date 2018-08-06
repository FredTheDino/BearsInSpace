#pragma once
//#include <stdio.h>
#include <stdlib.h>
//#include <cstring>
#include "bear_types.h"
#include "glad.h"

#define WINDOW_WIDTH 800
#define WINDOW_HEIGHT 600
#define ASPECT_RATIO ((float32) WINDOW_WIDTH) / WINDOW_HEIGHT

#define HALT_AND_CATCH_FIRE() ((int *)(void *)0)[0] = 1
#define ASSERT(expr) if (!(expr)) { HALT_AND_CATCH_FIRE(); }
#define KILOBYTE(n) 1024 * ((uint64)n);
#define MEGABYTE(n) 1024 * KILOBYTE((uint64)n);
#define GIGABYTE(n) 1024 * MEGABYTE((uint64)n);

// NOTE: For now, we don't expand this memory. 
// We could change it to expand the memory, but
// for now we have a limit on 1GB of permanent data
// and a temporary buffer. These cannot be expanded.
// So we have hard memory limits.
struct MemoryAllocation
{
	bool taken;
	uint32 size;
	MemoryAllocation *next_free;
};

struct GameMemory
{
	// For temporary allocations.
	// Clears itself when filled up.
	uint8 *temp_memory;
	uint8 *temp_at;
	uint64 temp_memory_size;
	
	// For permanent structures
	uint8 *static_memory;
	uint8 *static_at;
	uint64 static_memory_size;

	MemoryAllocation *free;
};

const uint32 spec_freq = 44100; // Move this.

// Input types
typedef float64 AxisValue;

enum ButtonState
{
	UP = 1, DOWN = 2, PRESSED = 4, RELEASED = 8
};

enum InputType
{
	KEY, MOUSE, CONTROLLER
};

#define INPUT_MAP_SIZE 256
#define MAX_AXIS    32
#define MAX_BUTTONS 32
#define CONTROLLER_AXIS_THRESHOLD .2
#define CONTROLLER_AXIS_FACTOR (1.0 / 32767.0)

struct OSFile
{
	int32 timestamp;
	uint64 size;
	void *data;
};

typedef void *(*AllocatorFunc)(uint64);
struct PLT
{
	void  (*log)	(string, int32, string, string);
	int32 (*print)	(string, ...); 

	float64 (*get_time) (void);

	OSFile (*read_file)	(string, AllocatorFunc);
	int32 (*last_write) (string);

	AxisValue (*axis_value) (string);
	ButtonState (*button_state) (string);
};

// Callbacks
typedef void (*StepFunc)(float32);
typedef void (*SoundFunc)(int16 *, int32);
typedef void (*ReloadFunc)(PLT, void *);
typedef void (*ReplaceFunc)();
typedef void (*InitFunc)(PLT, void *);
typedef void (*DestroyFunc)();

bool str_eq(const char *_a, const char *_b)
{
	char *a = (char *) _a;
	char *b = (char *) _b;
	bool eq = false;
	while (*a == *b)
	{
		if (*a == '\0')
		{
			eq = true;
			break;
		}
		a++;
		b++;
	}
	return eq;
}

uint64 str_len(const char *str)
{
	char *end = (char *) str;
	while (*end != '\0') end++;
	return (str - end); 
}

