// This is the windows specific CPP file. Compile this and 
// you'll compile the game for windows. It's as easy as that.

// TODO: Clean this file, a lot.
int win_printf(const char *format, ...);

bool running = true;

#include <windows.h>
#include <SDL2/SDL.h>

#include "bear_shared.h"
#include "bear_sdl_threads_plt.h"

#define LOG(type, ...) { win_printf("[%s:%d] %s :", __FILE__, __LINE__, type); win_printf(__VA_ARGS__); win_printf("\n"); }
#define ERROR_LOG(type, ...) LOG("ERROR:" #type, __VA_ARGS__)
#define PRINT(...) win_printf(__VA_ARGS__)
int32 win_printf(const char *format, ...);
void win_log(const char *file, int32 line, const char *type, const char *msg);

#include "bear_array_plt.cpp"
#include "bear_input.h"

#include "glad.c"

GameMemory mem;
PLT plt;

struct GameHandle
{
	StepFunc step;
	SoundFunc sound;

	ReloadFunc reload;
	ReplaceFunc replace;

	DestroyFunc destroy;

	// So the sound thread doesn't run when we unload.
	SDL_mutex *lock;
	bool first_load;

	int32 access_time;
	HMODULE lib;
};

static GameHandle game;


int32 win_printf(const char *format, ...)
{
	char buffer[256];
	va_list args;
	va_start(args, format);
	int len = vsprintf(buffer, format, args);
	OutputDebugString(buffer);
	return len;
}

void win_log(const char *file, int32 line, const char *type, const char *msg)
{
	win_printf("[%s:%d] %s  %s\n", file, line, type, msg);
}

float64 counter_frequency;
LARGE_INTEGER start;
float64 win_get_time()
{
	LARGE_INTEGER curr;
	QueryPerformanceCounter(&curr);
	return (curr.QuadPart - start.QuadPart) / counter_frequency;
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
	HALT_AND_CATCH_FIRE();
	return 0;
}

OSFile read_entire_file(const char *path, AllocatorFunc alloc)
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
	file.data = alloc(file.size + 1);
	fread(file.data, file.size, 1, disk);
	((uint8 *) file.data)[file.size] = 0; // Null terminate.
	fclose(disk);

	return file;
}

void win_random_read(const char *path, void *to, uint32 start_byte, uint32 read_length)
{
	FILE *disk = fopen(path, "rb");
	if (!disk)
	{
		*((uint8 *) to) = 0;
		return;
	}

	fseek(disk, start_byte, SEEK_END);
	size_t size = ftell(disk);

	fseek(disk, start_byte, SEEK_SET);
	size_t red = fread(to, read_length, 1, disk);
	if (!red)
	{
		HALT_AND_CATCH_FIRE();
	}
	fclose(disk);
}

bool load_libbear(GameHandle *handle)
{
	GameHandle game = *handle;
	const char *path = "bin/libbear.dll";
	const char *temp_path = "bin/libbear_temp.dll";
	int32 access_time = get_file_edit_time(path);
	if (access_time == game.access_time)
	{
		return false;
	}

	SDL_LockMutex(game.lock);
	if (game.lib)
	{
		game.replace();
		FreeLibrary(game.lib);
	}

	CopyFile(path, temp_path, false);

	game.lib = LoadLibrary(temp_path);
	if (!game.lib)
	{
		LOG("LIB LOAD", "Failed to load libgame.so!");
		win_printf("Windows Error Code: %d\n", GetLastError());
		return false;
	}
	StepFunc step = (StepFunc) GetProcAddress(game.lib, "step");
	if (!step)
	{
		goto LIB_LOAD_FAIL;
	}
	SoundFunc sound = (SoundFunc) GetProcAddress(game.lib, "sound");
	if (!sound)
	{
		goto LIB_LOAD_FAIL;
	}
	ReloadFunc reload = (ReloadFunc) GetProcAddress(game.lib, "reload");
	if (!reload)
	{
		goto LIB_LOAD_FAIL;
	}
	ReplaceFunc replace = (ReplaceFunc) GetProcAddress(game.lib, "replace");
	if (!replace)
	{
		goto LIB_LOAD_FAIL;
	}
	DestroyFunc destroy = (DestroyFunc) GetProcAddress(game.lib, "destroy");
	if (!destroy)
	{
		goto LIB_LOAD_FAIL;
	}

	if (game.first_load)
	{
		game.first_load = false;
		InitFunc init = (InitFunc) GetProcAddress(game.lib, "init");
		if (!init)
		{
			goto LIB_LOAD_FAIL;
		}
		init(plt, &mem);
	}

	PRINT("======== Reload! =========");
	game.access_time = access_time;

	game.step = step;
	game.sound = sound;
	game.reload = reload;
	game.replace = replace;
	game.destroy = destroy;

	game.reload(plt, &mem);

	*handle = game;
	SDL_UnlockMutex(game.lock);

	return true;

LIB_LOAD_FAIL:
	win_printf("[DLL-ERROR] %d\n", GetLastError());
	SDL_UnlockMutex(game.lock);
	return false;
}

void plt_audio_callback(void *userdata, uint8 *stream, int32 length)
{
	SDL_LockMutex(game.lock);
	game.sound((int16 *) stream, length / 2);
	SDL_UnlockMutex(game.lock);
}

int CALLBACK WinMain(
  HINSTANCE hInstance,
  HINSTANCE hPrevInstance,
  LPSTR     lpCmdLine,
  int       nCmdShow)
{
	plt.print = win_printf;
	plt.log = win_log;

	plt.read_file = read_entire_file;
	plt.random_file_read = win_random_read;
	plt.last_write = get_file_edit_time;

	plt.submit_work = send_work;
	
	plt.get_time = win_get_time;

	plt.axis_value = axis_value;
	plt.button_state = button_state;

	mem.static_memory_size = GIGABYTE(1);
	mem.static_memory = (uint8 *) VirtualAlloc(0, mem.static_memory_size, MEM_COMMIT | MEM_RESERVE, PAGE_READWRITE);
	if (mem.static_memory == nullptr)
	{
		win_printf("Failed to allocate game memory, gonna crash now. Sorry.\n");
		HALT_AND_CATCH_FIRE();
	}

	mem.temp_memory_size = GIGABYTE(1);
	mem.temp_memory = (uint8 *) VirtualAlloc(0, mem.temp_memory_size, MEM_COMMIT | MEM_RESERVE, PAGE_READWRITE);
	if (mem.temp_memory == nullptr)
	{
		win_printf("Failed to allocate game memory, gonna crash now. Sorry.\n");
		HALT_AND_CATCH_FIRE();
	}

	if (SDL_Init(SDL_INIT_AUDIO | SDL_INIT_VIDEO | SDL_INIT_GAMECONTROLLER) != 0)
	{
		LOG("INIT", "Unable to initalize SDL.");
		SDL_Quit();
		HALT_AND_CATCH_FIRE();
	}
	SDL_Window *window = SDL_CreateWindow(
			"Space Bears",
			SDL_WINDOWPOS_UNDEFINED,
			SDL_WINDOWPOS_UNDEFINED,
			WINDOW_WIDTH,
			WINDOW_HEIGHT,
			SDL_WINDOW_OPENGL
			);

	create_sdl_threads();

	SDL_RaiseWindow(window);
	
	// TODO: Use OpenGL 3.3, or newer. This is just to get hello triangle.
	SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 2);
	SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 1);
	SDL_GL_SetAttribute(SDL_GL_DOUBLEBUFFER, 1);
	SDL_GL_SetAttribute(SDL_GL_DEPTH_SIZE, 24);
	SDL_GL_CreateContext(window);

	SDL_GL_SetSwapInterval(0);

	SDL_AudioSpec audio_spec = {};
	audio_spec.callback = plt_audio_callback;
	audio_spec.freq = spec_freq;//spec_freq; // Is this dumb? Is 44100 better?
	audio_spec.format = AUDIO_S16; // Maybe too high rez?
	audio_spec.channels = 2; // This needs to be changeable.
	audio_spec.samples = 1024; // Ideally we want this as small as possible.
	auto audio_device = SDL_OpenAudioDevice(NULL, 0, &audio_spec, NULL, 0);
	if (!audio_device)
	{
		LOG("INIT", "Unable to open audio device");
		SDL_Quit();
		HALT_AND_CATCH_FIRE();
	}

	game.lock = SDL_CreateMutex();
	game.first_load = true;
	if (load_libbear(&game) == false)
	{
	  HALT_AND_CATCH_FIRE();
	}

	init_input();

	SDL_PauseAudioDevice(audio_device, 0);

	PRINT("Windows launch!");

	LARGE_INTEGER int_counter_frequency;
	QueryPerformanceFrequency(&int_counter_frequency);
	counter_frequency = (float64) int_counter_frequency.QuadPart;


	LARGE_INTEGER counter;
	LARGE_INTEGER last_counter;
	QueryPerformanceCounter(&last_counter);
	QueryPerformanceCounter(&start);
	float64 delta = 0.0f;
	while (running)
	{
		load_libbear(&game);

		//update_input();
		SDL_Event event;
		while (SDL_PollEvent(&event))
		{
			if (event.type == SDL_QUIT)
			{
				running = false;
			}
			else
			{
				handle_input_event(event);
			}
		}
		
		game.step(delta);

		SDL_GL_SwapWindow(window);
		
		// Timing
		QueryPerformanceCounter(&counter);
		int64 delta_counter = counter.QuadPart - last_counter.QuadPart;
		last_counter = counter;
		delta = (float64) delta_counter / counter_frequency;
	}

	game.destroy();
	destroy_input();
	VirtualFree(mem.temp_memory,   mem.temp_memory_size,   MEM_RELEASE);
	VirtualFree(mem.static_memory, mem.static_memory_size, MEM_RELEASE);
	
	SDL_CloseAudio();
	delete_sdl_threads();
	SDL_DestroyMutex(game.lock);
	SDL_Quit();

	return 0;
}


