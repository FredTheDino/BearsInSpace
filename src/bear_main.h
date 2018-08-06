#pragma once

#define PRINT(...) plt.print(__VA_ARGS__)
#define LOG(type, ...) \
	plt.print("[%s:%d] type :", __FILE__, __LINE__, type); \
	plt.print(__VA_ARGS__);\
	plt.print("\n");

// Array
#include "bear_array.cpp"

// Math
#include "math/bear_math.h"

// Audio
#include "audio/bear_audio.h"

// ECS
#include "ecs/bear_ecs.h"

// Physics
#include "physics/bear_physics.h"

// Clocks
#include "bear_clock.h"

// TODO: This can also be done a lot better.
namespace GFX
{
	struct MatrixProfile;
};


struct World
{
	// Audio, so the world can feel the beats.
	Audio audio;
	
	// The ECS, all storage in one place.
	ECS ecs;
	
	// Physics
	Physics phy;

	// A clock
	CLK clk;

	// GFX, maybe create a struct for this?
	// Maybe we should restructure this.
	Array<GFX::MatrixProfile> matrix_profiles;

};

#if 0
Audio 	*world_audio;
ECS 	*world_ecs;
Physics *world_phy;
CLK 	*world_clk;
#endif

Array<GFX::MatrixProfile> *world_matrix_profiles;

#define AXIS_VAL(name) (plt.axis_value(name))
#define B_STATE(name) (plt.button_state(name))
#define B_PRESSED(name) (plt.button_state(name) == ButtonState::PRESSED)
#define B_RELEASED(name) (plt.button_state(name) == ButtonState::RELEASED)
#define B_DOWN(name) (plt.button_state(name) == ButtonState::DOWN)
#define B_UP(name) (plt.button_state(name) == ButtonState::UP)

