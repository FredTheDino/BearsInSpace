// Exists just here.

#define BEAR_GAME
#include "bear_main.h"
#include "glad.c"

// This file is included in each platform specific file. 
// This file should _NOT HAVE ANY_ platform specific code.

// TODO: This is temporary. We shouldn't rely on printf 
// since windows dosn't allow it when not running a console 
// application.

void update(World *_world, float32 delta)
{
	// Initialize GLAD if necessary
	if (!glClear)
	{
		gladLoadGL();
	}
	
	world = _world;
	if (world->input.jump)
		DEBUG_LOG("It really dose!");
	ASSERT(true);
}

void draw(World *world)
{
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

int bear_main()
{
	DEBUG_LOG("Game started!");

	return 0;
}
