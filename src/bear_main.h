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

// TODO: Remove this in RELEASE
//
// Memory, checks some memory for you.

#define HALT_AND_CATCH_FIRE() ((int *)(void *)0)[0] = 1

// TODO: Clean this up
#ifdef BEAR_GAME
World *world;

// TODO: This is essentially duplicate from bear_memory.h
#define MALLOC2(type, num) (type *) \
	world->plt.malloc(__FILE__, __LINE__, sizeof(type) * num)
#define MALLOC1(type) (type *) \
	world->plt.malloc(__FILE__, __LINE__, sizeof(type))

#define GET_MACRO(_2, _1, NAME, ...) NAME
#define MALLOC(...) GET_MACRO(__VA_ARGS__, MALLOC2, MALLOC1) (__VA_ARGS__)

#define FREE(ptr) world->plt.free((void *)ptr)

#define REALLOC(ptr, size) world->plt.realloc(__FILE__, __LINE__, (void *) ptr, size)

#define DEBUG_LOG(message)  world->plt.log(__FILE__, __LINE__, "DEBUG", message)
#define ERROR_LOG(message)  world->plt.log(__FILE__, __LINE__, "ERROR", message)
#define LOG(message)		world->plt.log(__FILE__, __LINE__, "LOG", message)

#define PRINT(...)			world->plt.print(__VA_ARGS__)

#define ASSERT(check) ((check) ? (void)0 : assert_(__FILE__, __LINE__, #check))
void assert_(const char *file, uint32 line, const char *check)
{
	world->plt.log(file, line, "ASSERT", check);
	HALT_AND_CATCH_FIRE();
}

#else
World world;

#define LOG(message) DEBUG_LOG_(__FILE__, __LINE__, "LOG", message)
#define ERROR_LOG(message) DEBUG_LOG_(__FILE__, __LINE__, "ERROR", message)

#define DEBUG_LOG(message) DEBUG_LOG_(__FILE__, __LINE__, "DEBUG", message)
void DEBUG_LOG_(const char *file_name, const int line_number, const char *type, const char *message)
{
	// Replace this.
#ifdef WIN32
	win_printf("[%s:%d] %s: %s\n", file_name, line_number, type, message);
#else
	printf("[%s:%d] %s: %s\n", file_name, line_number, type, message);
#endif
}

#define ASSERT(check) ((check) ? (void)0 : ASSERT_(__FILE__, __LINE__, #check))
void inline ASSERT_(const char *file_name, const int line_number, const char *check)
{
	DEBUG_LOG_(file_name, line_number, "ASSERT", check);
	HALT_AND_CATCH_FIRE();
}

#endif
