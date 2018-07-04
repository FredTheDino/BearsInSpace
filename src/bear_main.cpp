// Exists just here.

#define GL_LOADED glClear


#define BEAR_GAME
#include "bear_main.h"
#include "bear_array.h"
#include "math/bear_math.h"
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
		run_tests();
	
	/*
	if (world->input.jump)
		DEBUG_LOG("Woot!");
	OSFile file = world->plt.read_file("res/tmp");
	world->plt.print("tmp: %s\n", (char *) file.data);
	world->plt.free_file(file);
	*/
}

void draw(World *world)
{
	// Initialize GLAD if necessary
	if (!GL_LOADED)
	{
		gladLoadGL();
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

