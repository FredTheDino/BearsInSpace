#pragma once

// Array Header
#include "bear_array.h"

// Math
#include "math/bear_math.h"

// Audio
#include "audio/bear_audio.h"
/*

// ECS
#include "ecs/bear_ecs.h"
#include "ecs/bear_ecs_init.cpp"

// Physics
#include "physics/bear_physics.h"
#include "physics/bear_physics_init.cpp"
*/

//#include "bear_input.h"

struct World
{
	// Physics
#if 0
	Physics phy;

	// The ECS, all storage in one place.
	ECS ecs;
#endif

	// Audio, so the world can feel the beats.
	Audio audio;
};



// A way to crash so we can bug track.

#if 0
#ifdef BEAR_GAME

//#include "bear_main_game.h"
#define PRINT(...)			plt->print(__VA_ARGS__)

#else

//#include "bear_main_plt.h"
//#include "bear_memory.h"

#endif

#endif
