// This file is compileda on each platform.
// This file should _NOT HAVE ANY_ platform specific code,
// that code should be placed on the platform layer.


#define BEAR_GAME
#include "bear_main.h"

// Misc
#include "bear_array.h"

// Math
#include "math/bear_math.h"

// GFX
#include "glad.c"
#include "bear_obj_loader.cpp"
#include "bear_image_loader.cpp"
#include "bear_gfx.h"
#define GL_LOADED glClear

// Audio
#include "audio/bear_audio.cpp"
#include "audio/bear_mixer.cpp"

// States
#include "bear_states.h"

// ECS
#include "ecs/bear_ecs.cpp"

// Tests
#include "bear_test.cpp"


#if 0
void update(float32 delta)
{
	// Temporary test code.
#if 0
	EntityID entity = add_entity(&world->ecs);
	Position comp;
	comp.type = C_POSITION;
	comp.position = {1.0f, 2.0f, 3.0f};

	Blargh blarg;
	blarg.type = C_BLARGH;
	blarg.a = 32;
	blarg.b = 11111111;

	add_components(&world->ecs, entity, &comp, &blarg);

	EntityID entity2 = add_entity(&world->ecs);
	add_component(&world->ecs, entity2, C_BLARGH, (BaseComponent *) &blarg);

	remove_entity(&world->ecs, entity);
	remove_entity(&world->ecs, entity2);

	run_system(S_HELLO_WORLD, world, delta);

	id = load_sound(&world->audio, "res/smack.wav");
	stockhousen = load_sound(&world->audio, "res/stockhausen.wav");
	play_music(&world->audio, stockhousen, 1.0f, 1.0f, true);
	world->audio.left = {-1.0f, 0.0f, 0.0f};
	world->audio.position = { (float32) sin(t / 2.0f), 0.0f, 2.0f};
	t += delta;

	static bool pressed_jump = false;
	if (world->input.jump)
	{
		if (!pressed_jump)
		{
			pressed_jump = true;
			play_sound(&world->audio, id, 0.5f, 0.5f);
		}
	}
	else
	{
		pressed_jump = false;
	}
#endif

}

void draw()
{
}
#endif

extern "C"
void step(World *_world, float32 delta)
{
	world = _world;
	
	// Initialize GLAD if necessary
	if (!GL_LOADED)
	{
		gladLoadGL();
	}

	if (should_run_tests)
	{
		run_tests();
#if 0
		// Test code.
		Mesh mesh = load_mesh("res/monkey.obj");
		free_mesh(mesh);
#endif
	}

	// Enter first state
	if (!is_valid_state(world->state))
		world->next_state = test_state;
	
	// Enter new state
	if (is_valid_state(world->next_state))
	{
		// Call exit function on old state
		if (is_valid_state(world->state))
			world->state.exit();
		// Set next state as current
		world->state = world->next_state;
		world->next_state = {};
		world->state.enter();
	}


	// Update
	world->state.update(delta);
	
	// Draw
	glClearColor(0.1f, 0.1f, 0.1f, 1.0f);
	glClear(GL_COLOR_BUFFER_BIT);

	world->state.draw();

	
	//update(delta);
	//draw();
}


