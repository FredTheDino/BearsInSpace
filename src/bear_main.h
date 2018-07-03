#pragma once
#include <stdlib.h>
#include <stdio.h>
#include "glad.h"

#include "bear_types.h"

struct PLAT
{
	void *(*malloc)	(const char *, uint32, uint64);
	void  (*free)	(void *);
	void *(*realloc)(const char *, uint32, void *, uint64);

	void  (*print)	(const char *, int32, const char *, const char *);
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
	PLAT plt;

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


#ifdef BEAR_GAME
World *world;

#define DEBUG_LOG(message)  world->plt.print(__FILE__, __LINE__, "DEBUG", message)
#define ERROR_LOG(message)  world->plt.print(__FILE__, __LINE__, "ERROR", message)
#define LOG(message)		world->plt.print(__FILE__, __LINE__, "LOG", message)

#define ASSERT(check) ((check) ? (void)0 : assert_(__FILE__, __LINE__, #check))
void assert_(const char *file, uint32 line, const char *check)
{
	world->plt.print(file, line, "ASSERT", check);
	exit(-1);
}

#else
World world;

#define LOG(message) DEBUG_LOG_(__FILE__, __LINE__, "LOG", message)
#define ERROR_LOG(message) DEBUG_LOG_(__FILE__, __LINE__, "ERROR", message)

#define DEBUG_LOG(message) DEBUG_LOG_(__FILE__, __LINE__, "DEBUG", message)
void DEBUG_LOG_(const char *file_name, const int line_number, const char *type, const char *message)
{
	// Replace this.
	printf("[%s:%d] %s: %s\n", file_name, line_number, type, message);
}

#define ASSERT(check) ((check) ? (void)0 : ASSERT_(__FILE__, __LINE__, #check))
void inline ASSERT_(const char *file_name, const int line_number, const char *check)
{
	DEBUG_LOG_(file_name, line_number, "ASSERT", check);
	exit(-1);
}

#endif

#include "array.h"
