// This is the windows specific CPP file. Compile this and 
// you'll compile the game for windows. It's as easy as that.
int win_printf(const char *format, ...);

#include <windows.h>
#include <SDL2/SDL.h>
#include "glad.h"
#include "glad.c"
#include "bear_main.h"
#include "bear_memory.h"

// Group this up?
UpdateFunc game_update;
DrawFunc game_draw;

int32 last_access_time = 0;

HMODULE handle;



int32 win_printf(const char *format, ...)
{
	char buffer[256];
	va_list args;
	va_start(args, format);
	int len = vsprintf(buffer, format, args);
	OutputDebugString(buffer);
	return len;
}

int32 get_last_file_access(const char *path)
{
	FILETIME last_write;
	HANDLE file_handle = CreateFileA(
			path,
			GENERIC_READ,
			0,
			NULL,
			OPEN_EXISTING,
			FILE_ATTRIBUTE_NORMAL,
			NULL);
	if (!file_handle)
	{
		goto WINDOWS_FILE_ERROR;
	}

	if (!GetFileTime(file_handle, NULL, NULL, &last_write))
	{
		goto WINDOWS_FILE_ERROR;
	}

	CloseHandle(file_handle);
	return last_write.dwLowDateTime;

WINDOWS_FILE_ERROR:
	CloseHandle(file_handle);
	return -1;
}

bool load_libbear()
{
	const char *path = "libbear.dll";
	const char *temp_path = "libbear_temp.dll";
	int32 last = get_last_file_access(path);
	if (last == last_access_time)
	{
		return false;
	}
	if (handle)
	{
		FreeLibrary(handle);
	}
	CopyFile(path, temp_path, false);
	handle = LoadLibrary(temp_path);
	if (!handle)
	{
		win_printf("[DL-ERROR] %d\n", GetLastError());
		DEBUG_LOG("Failed to load libgame.so!");
		return false;
	}
	UpdateFunc update = (UpdateFunc) GetProcAddress(handle, "update");
	if (!update)
	{
		win_printf("[DL-ERROR] %d\n", GetLastError());
		DEBUG_LOG("Failed to load game_update!");
		return false;
	}
	DrawFunc draw = (DrawFunc) GetProcAddress(handle, "draw");
	if (!draw)
	{
		win_printf("[DL-ERROR] %d\n", GetLastError());
		DEBUG_LOG("Failed to load game_draw!");
		return false;
	}

	DEBUG_LOG("Reload!");
	last_access_time = last;
	game_update = update;
	game_draw = draw;
	return true;

}

#ifdef asdas //__DEBUG 
int main(int varc, char *varv[])
#else
int CALLBACK WinMain(
  HINSTANCE hInstance,
  HINSTANCE hPrevInstance,
  LPSTR     lpCmdLine,
  int       nCmdShow
)
#endif
{
	world.plt.print = DEBUG_LOG_;
	world.plt.malloc = malloc_;
	world.plt.free = free_;
	world.plt.realloc = realloc_;

	if (load_libbear() == false)
	{
		return(-1);
	}
	//has_file_changed();

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

	world.gl.clear = glClear;
	world.gl.clear_color = glClearColor;
	world.gl.begin = glBegin;
	world.gl.end = glEnd;
	world.gl.color3f = glColor3f;
	world.gl.vertex2f = glVertex2f;

	DEBUG_LOG("Window launch!");

	bool running = true;
	while (running)
	{
		load_libbear();

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
	return 0;
	//return(bear_main());
}

