#pragma once
#include <stdio.h>
#include "glad.h"

// Types
typedef const char *string;

typedef char	int8;
typedef short	int16;
typedef int		int32;
typedef long	int64;

typedef unsigned char	uint8;
typedef unsigned short	uint16;
typedef unsigned int	uint32;
typedef unsigned long	uint64;

typedef float  float32;
typedef double float64;

#ifdef __DEBUG

#define DEBUG_LOG(message) DEBUG_LOG_(__FILE__, __LINE__, message)
void DEBUG_LOG_(const char *file_name, const int line_number, const char *message)
{
	printf("[%s:%d] %s\n", file_name, line_number, message);
}

#else

#define DEBUG_LOG(message) 

#endif

struct GL
{
#define PROC(name) PFNGL##name##PROC
	PROC(CLEAR) 
		clear;
	PROC(CLEARCOLOR) 
		clear_color;
	PROC(BEGIN) 
		begin;
	PROC(END)	
		end;
	PROC(COLOR3F)	
		color3f;
	PROC(VERTEX2F)	
		vertex2f;
};

struct World
{
	struct Input
	{
		bool jump;
	} input;

	// GL functions.
	GL gl;
} world;

typedef void (*UpdateFunc)(World *, float32);
typedef void (*DrawFunc)(World *);

extern "C"
void update(World *world, float32 delta);
extern "C"
void draw(World *world);
