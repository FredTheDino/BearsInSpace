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

void update(World *_world, float32 delta)
{
	world = _world;

	if (should_run_tests)
	{
		run_tests();
	}
}

void draw(World *world)
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
	glColor3f(1.0f, 0.0f, 1.0f);
	glVertex2f(0.0f, 0.5f);
	glColor3f(1.0f, 0.0f, 0.0f);
	glVertex2f(0.5f, -0.5f);
	glEnd();
}

