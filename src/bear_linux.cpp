#include "bear_main.h"
#include "bear_memory.h"
#include <SDL2/SDL.h>
#include <sys/stat.h>
#include <dlfcn.h>
#include "glad.c"

UpdateFunc game_update;
DrawFunc game_draw;
int32 last_edit;
int32 count_down = -1;
void *handle;

const char *path = "./bin/libgame.so";
bool has_file_changed()
{
	struct stat attr;
	// Check for success
	stat(path, &attr);
	int mtime = attr.st_mtime;
	if (mtime != last_edit)
	{
		last_edit = mtime;
		return true;
	}
	return false;
}

bool load_libgame()
{
	// Check if it has rebuilt, assume it has
	DEBUG_LOG("Reload!");

	if (handle)
	{
		dlclose(handle);
	}

	handle = dlopen(path, RTLD_NOW);
	if (!handle)
	{
		printf("[DL-ERROR] %s\n", dlerror());
		DEBUG_LOG("Failed to load libgame.so!");
		return false;
	}
	game_update = (UpdateFunc) dlsym(handle, "update");
	game_draw   = (DrawFunc)   dlsym(handle, "draw");
	return true;
}

int main(int varc, char *varv[])
{
	world.plt.print = DEBUG_LOG_;
	world.plt.malloc = malloc_;
	world.plt.free = free_;
	world.plt.realloc = realloc_;

	if (load_libgame() == false)
	{
		return(-1);
	}
	has_file_changed();

	if (SDL_Init(SDL_INIT_EVERYTHING) != 0)
	{
		DEBUG_LOG("Unable to initalize SDL.");
		SDL_Quit();
		return(-1);
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
	//SDL_GLContext glcontext = // Do I need this?
	SDL_GL_CreateContext(window);

	if (gladLoadGL() == 0)
	{
		DEBUG_LOG("Unable to load OpenGL.");
		SDL_Quit();
		return(-1);
	}

	DEBUG_LOG("Linux launch!");

	bool running = true;
	while (running)
	{
		if (has_file_changed())
		{
			DEBUG_LOG("====== Reloading!");
			count_down = 60;
		}

		if (count_down >= 0)
		{
			count_down--;
		}

		if (count_down == 0)
		{
			load_libgame();
		}

		SDL_Event event;
		while (SDL_PollEvent(&event))
		{
			if (event.type == SDL_QUIT)
			{
				running = false;
			}
			if (event.type == SDL_KEYDOWN)
			{
				world.input.jump = true;
			}
			if (event.type == SDL_KEYUP)
			{
				world.input.jump = false;
			}
		}
		
		game_update(&world, 0.1f);
		game_draw(&world);

		SDL_GL_SwapWindow(window);
	}
	SDL_Quit();

	return(1);
}
