// Exists just here.

#define GL_LOADED glClear

#define BEAR_GAME
#include "bear_main.h"
#include "bear_array.h"
#include "math/bear_math.h"
#include "bear_obj_loader.cpp"
#include "bear_image_loader.cpp"
#include "glad.c"

#include "bear_test.cpp"

// This file is included in each platform specific file. 
// This file should _NOT HAVE ANY_ platform specific code.

// TODO: This is temporary. We shouldn't rely on printf 
// since windows dosn't allow it when not running a console 
// application.


void update(float32 delta)
{

	if (should_run_tests)
	{
		run_tests();
	}
}

void draw()
{
	// Initialize GLAD if necessary
	if (!GL_LOADED)
	{
		gladLoadGL();
		Mesh mesh = load_mesh("res/monkey.obj");
		free_mesh(mesh);
	}

	glClearColor(0.1f, 0.1f, 0.1f, 1.0f);
	glClear(GL_COLOR_BUFFER_BIT);

	glBegin(GL_TRIANGLES);
	glColor3f(0.0f, 0.0f, 1.0f);
	glVertex2f(-0.5f, -0.5f);
	glColor3f(0.0f, 1.0f, 0.0f);
	glVertex2f(0.0f, 0.5f);
	glColor3f(1.0f, 0.0f, 0.0f);
	glVertex2f(0.5f, -0.5f);
	glEnd();
}

extern "C"
void step(World *_world, float32 delta)
{
	world = _world;
	update(delta);
	draw();
}

#define PI 3.1419f

uint32 spec_freq = 44100;
float32 t = 0;

extern "C"
void sound(float32 *buffer, int32 num_samples)
{
	while (num_samples)
	{
		float32 sample = sin(t * 2 * 442 * PI);
		sample = 0.0f; // This is so it doesn't get annoying.
		t += 1.0f / spec_freq;
		float32 left_panning = sin(t * 2 * PI * 0.1f) * 0.5f;
		*buffer++ = sample * left_panning;
		*buffer++ = sample * (1.0f - left_panning);
		num_samples -= 2;
	}
}


