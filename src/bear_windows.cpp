// This is the windows specific CPP file. Compile this and 
// you'll compile the game for windows. It's as easy as that.
int win_printf(const char *format, ...);

#include <windows.h>
#include <SDL2/SDL.h>

#include "bear_main.h"
#include "glad.c"

// Group this up?
UpdateFunc game_update;
DrawFunc game_draw;

// This is ugly... I know.
MemoryAllocation __mem[1024];

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

int32 get_file_edit_time(const char *path)
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
	int32 last = get_file_edit_time(path);
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

#define PI 3.141592653f

float32 t;
uint32 tone_hz = 441;
uint32 spec_freq = 44100;

#include <stdlib.h>

void plt_audio_callback(void *userdata, uint8 *_stream, int32 _length)
{
	float32 *stream = (float32 *)_stream;
	int32 length = _length / 4;

	while (0 < length)
	{
		float32 sample = sin(t * 2.0f * PI * tone_hz) * 28000;
		t += 1.0f / spec_freq;/// (float32) (spec_freq / tone_hz);
		*(stream++) = sample;
		*(stream++) = sample;
		length -= 2;
	}
}

void free_file(OSFile file)
{
	if (file.data)
	{
		FREE(file.data);
		file.data = 0;
	}
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
	world.plt.malloc = malloc_;
	world.plt.free = free_;
	world.plt.realloc = realloc_;

	world.plt.print = win_printf;
	world.plt.log = debug_log_;

	world.plt.read_file = read_entire_file;
	world.plt.free_file = free_file;
	world.plt.last_write = get_file_edit_time;

	world.__mem = (MemoryAllocation *)(void *)__mem;

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

	//
	// Display stuff.
	// 
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
	SDL_GL_CreateContext(window);

	if (gladLoadGL() == 0)
	{
		DEBUG_LOG("Unable to load OpenGL.");
		SDL_Quit();
		return(-1);
	}

	// 
	//	Audio Stuff.
	//
	
	SDL_AudioSpec audio_spec = {};
	audio_spec.callback = plt_audio_callback;
	audio_spec.freq = spec_freq; // Is this dumb? Is 44100 better?
	audio_spec.format = AUDIO_F32; // Maybe too high rez?
	audio_spec.channels = 2; // This needs to be changeable.
	audio_spec.samples = 4096; // Ideally we want this as small as possible.
	if (SDL_OpenAudioDevice(NULL, 0, &audio_spec, NULL, 0) == 0)
	{
		DEBUG_LOG("Unable to load audio.");
		SDL_Quit();
		return(-1);
	}

	SDL_PauseAudio(0);
	
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
	SDL_CloseAudio();
	SDL_Quit();

	return(1);
	return 0;
	//return(bear_main());
}

