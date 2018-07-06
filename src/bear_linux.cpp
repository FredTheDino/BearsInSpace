#include <SDL2/SDL.h>
#include <sys/stat.h>
#include <dlfcn.h>
#include <unistd.h>

#include "bear_main.h"
#include "glad.c"

UpdateFunc game_update;
DrawFunc game_draw;

int32 last_edit;
void *handle;

MemoryAllocation __mem[1024];

int32 get_file_edit_time(const char *path)
{
	struct stat attr;
	// Check for success
	if (stat(path, &attr))
	{
		return -1;
	}
	return attr.st_ctime;
}

OSFile read_entire_file(const char *path)
{
	OSFile file = {};
	file.timestamp = get_file_edit_time(path);
	if (file.timestamp == -1)
	{
		return file;
	}

	FILE *disk = fopen(path, "rb");
	if (!disk)
	{
		return file;
	}

	fseek(disk, 0, SEEK_END);
	file.size = ftell(disk);
	fseek(disk, 0, SEEK_SET);
	file.data = malloc_("FILE IO", 0, file.size + 1);
	fread(file.data, file.size, 1, disk);
	((uint8 *) file.data)[file.size] = 0; // Null terminate.
	fclose(disk);

	return file;
}

void free_file(OSFile file)
{
	if (file.data)
	{
		FREE(file.data);
		file.data = 0;
	}
}


bool load_libgame()
{
	// Check if it has rebuilt, assume it has
	const char *path = "./bin/libbear.so";

	int32 new_last_edit = get_file_edit_time(path);
	if (new_last_edit == last_edit)
	{
		return false;
	}

	void *temp_handle = dlopen(path, RTLD_NOW);
	if (!temp_handle)
	{
		printf("[DL-ERROR] %s\n", dlerror());
		DEBUG_LOG("Failed to load libbear.so!");
		return false;
	}

	dlerror();
	dlsym(temp_handle, "update");
	auto error_code = dlerror();
	if (error_code)
	{
		return false;
	}
	dlclose(temp_handle);

	if (handle)
	{
		dlclose(handle);
	}
	handle = dlopen(path, RTLD_NOW);

	UpdateFunc new_game_update = (UpdateFunc) dlsym(handle, "update");
	DrawFunc   new_game_draw   = (DrawFunc)   dlsym(handle, "draw");

	if (!new_game_draw)
	{
		return false;
		DEBUG_LOG("Failed to load draw");
	}
	game_draw = new_game_draw;
	if (!new_game_update)
	{
		return false;
		DEBUG_LOG("Failed to load update");
	}
	game_update = new_game_update;

	DEBUG_LOG("Reload!");
	last_edit = new_last_edit;
	return true;
}

int main(int varc, char *varv[])
{
	world.plt.malloc = malloc_;
	world.plt.free = free_;
	world.plt.realloc = realloc_;

	world.plt.log = DEBUG_LOG_;
	world.plt.print = printf;

	world.plt.read_file = read_entire_file;
	world.plt.free_file = free_file;
	world.plt.last_write = get_file_edit_time;

	world.__mem = (MemoryAllocation *)(void *)__mem;

	if (!load_libgame())
	{
		return(-1);
	}

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
		load_libgame();

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

	//TODO: Make this a function so each platform layer can call it.
	// Memory check
	if (world.__mem_length != 0)
	{
		for (uint8 i = 0; i < world.__mem_length; i++)
		{
			MemoryAllocation alloc = world.__mem[i];
			printf("[MEM] Not freed (%s:%d)", alloc.file, alloc.line);
		}
	}

	return 0;
}
