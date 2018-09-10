// This file is compileda on each platform.
// This file should _NOT HAVE ANY_ platform specific code,
// that code should be placed on the platform layer.

struct GameMemory;
GameMemory *mem;

#include "bear_shared.h"
PLT plt;
#include "bear_memory.cpp"

// Random! :P
#include "bear_random.h"

#include "bear_main.h"
World *world;

// Clocks
#include "bear_clock.cpp"

// Audio
#include "audio/bear_audio.cpp"
#include "audio/bear_mixer.cpp"

// Utility
//#include "bear_utility.h"

// GFX
#include "glad.c"
#define GL_LOADED glClear
#include "bear_gfx.h"

// TODO: REMOVE THIS SHIT
ShaderProgram program;

// ECS
#include "ecs/bear_ecs.cpp"

// Physics
#include "physics/bear_physics.cpp"

// Assets
#include "bear_loader.cpp"

#include "world/bear_world_gen.cpp"

// Font rendering... It depends on wierd stuff...
#include "gfx/bear_font.cpp"

// Draw function for ECS (should preferably be the last include)
#include "gfx/bear_draw_ecs.h"

// Enter APP
extern "C"
void init(PLT _plt, GameMemory *_mem)
{
	mem = _mem;
	plt = _plt;
	world = static_push_struct(World); // This must allways be the first allocation.

	world->rng = seed(34565432654323);
  
	if (!GL_LOADED)
    {
		auto error = gladLoadGL();
		ASSERT(error);
		glEnable(GL_DEPTH_TEST);
	}
	
	init_ecs(&world->ecs);
	init_phy(&world->phy);

	world->camera = create_camera(create_perspective_projection(PI / 4, ASPECT_RATIO, .01f, 100.0f));
	
	// Output graphics
	world->output_buffer = create_frame_buffer(temp_array<uint32>({ GL_COLOR_ATTACHMENT0 }), WINDOW_WIDTH, WINDOW_HEIGHT);
	world->output_vb = create_vertex_buffer(temp_array<float32>({ -1, -1, 1, -1, -1, 1, 1, -1, 1, 1, -1, 1 }));
	world->output_quad = create_vertex_array(
		temp_array<VertexAttribute>({
				{ world->output_vb, 0, 2, GL_FLOAT }
		}));
	world->output_program = create_shader_program(temp_array<ShaderInfo>({ { GL_VERTEX_SHADER, "src/shader/post.vert" },
																		   { GL_FRAGMENT_SHADER, "src/shader/post.frag" } }));

	// -- START TEMP PROGRAM -- //
	Array<ShaderInfo> shader_info = temp_array<ShaderInfo>({
		{ GL_VERTEX_SHADER, "src/shader/simple.vert" },
		{ GL_FRAGMENT_SHADER, "src/shader/simple.frag" }
	});

	program = create_shader_program(shader_info);

	// -- END TEMP PROGRAM -- //
}

// Reload the library.
extern "C"
void reload(PLT _plt, GameMemory *_mem)
{
	if (!world)
	{
		mem = _mem;
		plt = _plt;
		world = (World *) ((MemoryAllocation *) mem->static_memory + 1);
	}
	
	if (!GL_LOADED)
	{
		gladLoadGL();
	}
	
	init_font_rendering();

	start_loader();
	
	init_gfx_debug();
	
	clear_ecs(&world->ecs, &world->phy);
}

// Exit APP
extern "C"
void destroy()
{
	delete_shader_program(world->output_program);
	delete_vertex_array(world->output_quad);
	delete_vertex_buffer(world->output_vb);
	delete_frame_buffer(world->output_buffer);
}

// Exit the library
extern "C"
void replace()
{
	PRINT("MEM watermark: %d\n", get_static_memory_watermark());
	stop_loader();
	destroy_gfx_debug();
}

extern "C"
void step(float32 delta)
{
	update_assets();

	run_system(S_PHYSICS, world, minimum(delta, 1.0f / 30.0f));
}
