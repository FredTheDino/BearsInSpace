// This is the windows specific CPP file. Compile this and 
// you'll compile the game for windows. It's as easy as that.
int win_printf(const char *format, ...);

#include <windows.h>
#include <SDL2/SDL.h>

#include "bear_main.h"
#include "glad.c"

// Group this up?

typedef void (*StepFunc)(World *, float32);
typedef void (*SoundFunc)(int16 *, int32);

struct GameHandle
{
	StepFunc step;
	SoundFunc sound;

	// So the sound thread doesn't run when we unload.
	SDL_mutex *lock;

	int32 access_time;
	HMODULE lib;
};

static GameHandle game;

// This is ugly... I know.
MemoryAllocation __mem[1024];


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
		FreeLibrary(game.lib);
	}
	CopyFile(path, temp_path, false);
	game.lib = LoadLibrary(temp_path);
	if (!game.lib)
	{
		win_printf("[DLL-ERROR] %d\n", GetLastError());
		DEBUG_LOG("Failed to load libgame.so!");
		return false;
	}
	StepFunc step = (StepFunc) GetProcAddress(game.lib, "step");
	if (!step)
	{
		win_printf("[DLL-ERROR] %d\n", GetLastError());
		DEBUG_LOG("Failed to load game_update!");
		return false;
	}
	SoundFunc sound = (SoundFunc) GetProcAddress(game.lib, "sound");
	if (!sound)
	{
		win_printf("[DLL-ERROR] %d\n", GetLastError());
		DEBUG_LOG("Failed to load game_draw!");
		return false;
	}

	DEBUG_LOG("Reload!");
	game.access_time = access_time;

	game.step = step;

	game.sound = sound;
	SDL_UnlockMutex(game.lock);

	*handle = game;
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

void free_file(OSFile file)
{
	if (file.data)
	{
		FREE(file.data);
		file.data = 0;
	}
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
	world.plt.malloc = malloc_;
	world.plt.free = free_;
	world.plt.realloc = realloc_;

	world.plt.print = win_printf;
	world.plt.log = debug_log_;

	world.plt.read_file = read_entire_file;
	world.plt.free_file = free_file;
	world.plt.last_write = get_file_edit_time;

	world.__mem = (MemoryAllocation *)(void *)__mem;
	world.audio = {};
	world.audio.buffers = MALLOC2(AudioBuffer, BEAR_MAX_AUDIO_BUFFERS);
	world.audio.sources = MALLOC2(AudioSource, BEAR_MAX_AUDIO_SOURCES);

	init_ecs(&world);

	game.lock = SDL_CreateMutex();
	if (load_libbear(&game) == false)
	{
		return(-1);
	}
	//has_file_changed();

	if (SDL_Init(SDL_INIT_AUDIO | SDL_INIT_VIDEO | SDL_INIT_GAMECONTROLLER) != 0)
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
	SDL_GL_SetAttribute(SDL_GL_DOUBLEBUFFER, 1);
	SDL_GL_SetAttribute(SDL_GL_DEPTH_SIZE, 24);
	SDL_GL_CreateContext(window);

	SDL_GL_SetSwapInterval(0);

	if (gladLoadGL() == 0)
	{
		DEBUG_LOG("Unable to load OpenGL.");
		SDL_Quit();
		return(-1);
	}

	// 
	//	Audio Stuff.
	//
	
#if 0
	uint32 wav_length;
	uint8 *wav_buffer;

	if (SDL_LoadWAV("res/sine.wav", NULL, &wav_buffer, &wav_length) == 0)
	{
		DEBUG_LOG("Unable to load WAV file.");
		SDL_Quit();
		return(-1);
	}
#endif

	SDL_AudioSpec audio_spec = {};
	audio_spec.callback = plt_audio_callback;
	audio_spec.freq = spec_freq;//spec_freq; // Is this dumb? Is 44100 better?
	audio_spec.format = AUDIO_S16; // Maybe too high rez?
	audio_spec.channels = 2; // This needs to be changeable.
	audio_spec.samples = 1024; // Ideally we want this as small as possible.
	auto audio_device = SDL_OpenAudioDevice(NULL, 0, &audio_spec, NULL, 0);
	if (!audio_device)
	{
		DEBUG_LOG("Unable to open audio device");
		SDL_Quit();
		return(-1);
	}

	SDL_PauseAudioDevice(audio_device, 0);
	
	DEBUG_LOG("Window launch!");

	world.running = true;

	LARGE_INTEGER counter_frequency = counter_frequency;
	QueryPerformanceFrequency(&counter_frequency);
	LARGE_INTEGER counter;
	LARGE_INTEGER last_counter;
	LARGE_INTEGER start;
	QueryPerformanceCounter(&start);
 
	QueryPerformanceCounter(&last_counter);
	while (world.running)
	{
		

		load_libbear(&game);

		SDL_Event event;
		while (SDL_PollEvent(&event))
		{
			if (event.type == SDL_QUIT)
			{
				world.running = false;
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
		
		game.step(&world, world.clk.delta);

		SDL_GL_SwapWindow(window);
		
		// Timing
		QueryPerformanceCounter(&counter);
		int64 delta_counter = counter.QuadPart - last_counter.QuadPart;
		last_counter = counter;
		world.clk.delta = (float64) delta_counter / (float64) counter_frequency.QuadPart;
		world.clk.time = (float64) (counter.QuadPart - start.QuadPart) / (float64) counter_frequency.QuadPart;
	}
	SDL_DestroyMutex(game.lock);
	SDL_CloseAudio();
	SDL_Quit();

	FREE(world.audio.sources);
	FREE(world.audio.buffers);

	destroy_ecs(&world);

	check_for_leaks();
	return 0;
}

