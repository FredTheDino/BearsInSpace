// This file is included in each platform specific file. 
// This file should _NOT HAVE ANY_ platform specific code.
#include <SDL2/SDL.h>
#include "glad.h"
#include "glad.c"

// TODO: This is temporary. We shouldn't rely on printf 
// since windows dosn't allow it when not running a console 
// application.
#include <stdio.h>
#ifdef __DEBUG

#define DEBUG_LOG(message) DEBUG_LOG_(__FILE__, __LINE__, message)
void DEBUG_LOG_(const char *file_name, const int line_number, const char *message)
{
	printf("[%s:%d] %s\n", file_name, line_number, message);
}

#else

#define DEBUG_LOG(message) 

#endif

int bear_main()
{
	DEBUG_LOG("Game started!");

	if (SDL_Init(SDL_INIT_EVERYTHING) != 0)
	{
		DEBUG_LOG("Unable to initalize SDL.");
		SDL_Quit();
		return -1;
	}

	SDL_Window *window = SDL_CreateWindow(
			"Space Bears",
			SDL_WINDOWPOS_UNDEFINED,
			SDL_WINDOWPOS_UNDEFINED,
			400,
			300,
			SDL_WINDOW_OPENGL
			);

	SDL_RaiseWindow(window);
	
	// TODO: Use OpenGL 3.3, or newer. This is just to get hello triangle.
	SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 2);
	SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 1);
	SDL_GL_SetSwapInterval(1);
	SDL_GL_SetAttribute(SDL_GL_DOUBLEBUFFER, 1);
	SDL_GL_SetAttribute(SDL_GL_DEPTH_SIZE, 24);
	SDL_GLContext glcontext = SDL_GL_CreateContext(window);

	if (gladLoadGL() == 0)
	{
		DEBUG_LOG("Unable to load OpenGL.");
		SDL_Quit();
		return -1;
	}

	bool running = true;
	while (running)
	{
		SDL_Event event;
		while (SDL_PollEvent(&event))
		{
			if (event.type == SDL_QUIT)
			{
				running = false;
			}
		}

		glClearColor(0.1f, 0.1f, 0.1f, 1.0f);
		glClear(GL_COLOR_BUFFER_BIT);

		glBegin(GL_TRIANGLES);
		glColor3f(1.0f, 0.0f, 0.0f);
		glVertex2f(-0.5f, -0.5f);

		glColor3f(0.0f, 1.0f, 0.0f);
		glVertex2f(0.0f, 0.5f);

		glColor3f(0.0f, 0.0f, 1.0f);
		glVertex2f(0.5f, -0.5f);
		glEnd();

		SDL_GL_SwapWindow(window);
	}

	SDL_Quit();
	return 0;
}
