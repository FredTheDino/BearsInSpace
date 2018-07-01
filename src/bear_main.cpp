#include "bear_main.h"
// This file is included in each platform specific file. 
// This file should _NOT HAVE ANY_ platform specific code.

// TODO: This is temporary. We shouldn't rely on printf 
// since windows dosn't allow it when not running a console 
// application.

void update(World *world, float32 delta)
{
	if (world->input.jump)
		DEBUG_LOG("It works!");
}

void draw(World *world)
{
	GL gl = world->gl;
	gl.clear_color(0.1f, 0.1f, 0.1f, 1.0f);
	gl.clear(GL_COLOR_BUFFER_BIT);

	gl.begin(GL_TRIANGLES);
	gl.color3f(1.0f, 0.0f, 0.0f);
	gl.vertex2f(-0.5f, -0.5f);

	gl.color3f(0.0f, 1.0f, 0.0f);
	gl.vertex2f(0.0f, 0.5f);

	gl.color3f(0.0f, 0.0f, 1.0f);
	gl.vertex2f(0.5f, -0.5f);
	gl.end();


}

int bear_main()
{
	DEBUG_LOG("Game started!");

	return 0;
}
