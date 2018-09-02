#pragma once

#define PRINT(...) plt.print(__VA_ARGS__)
#define LOG(type, ...) \
	{\
	plt.print("[%s:%d] %s :", __FILE__, __LINE__, type); \
	plt.print(__VA_ARGS__);\
	plt.print("\n");\
	}
#define ERROR_LOG(type, ...) LOG("ERROR:" #type, __VA_ARGS__)

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

// Loader
// #include "bear_loader.h" I kinda want this here. But GFX is needed.

// Graphics
#include "gfx/bear_vertex_array.h"
#include "gfx/bear_vertex_buffer.h"
#include "gfx/bear_shader_program.h"
#include "gfx/bear_frame_buffer.h"

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

	// Holds global matrix profiles
	Array<GFX::MatrixProfile> matrix_profiles;
	
	// Graphics output
	GFX::FrameBuffer output_buffer;
	GFX::VertexBuffer output_vb;
	GFX::VertexArray output_quad;
	GFX::ShaderProgram output_program;
};

#define AXIS_VAL(name) (plt.axis_value(name))
#define B_STATE(name) (plt.button_state(name))
#define B_PRESSED(name) (plt.button_state(name) == ButtonState::PRESSED)
#define B_RELEASED(name) (plt.button_state(name) == ButtonState::RELEASED)
#define B_DOWN(name) (plt.button_state(name) == ButtonState::DOWN)
#define B_UP(name) (plt.button_state(name) == ButtonState::UP)

