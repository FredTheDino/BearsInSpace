#include <SDL2/SDL.h>
#include <sys/stat.h>
#include <dlfcn.h>
#include <unistd.h>
#include <time.h>
#include <stdio.h>

#include "bear_shared.h"
#define LOG(type, ...) { printf("[%s:%d] %s :", __FILE__, __LINE__, type); printf(__VA_ARGS__); printf("\n"); }
#define ERROR_LOG(type, ...) LOG("ERROR:" #type, __VA_ARGS__)
#define PRINT(...) printf(__VA_ARGS__)

void linux_log(string file, int32 line, string type, string msg)
{
	printf("[%s:%d] %s %s\n", file, line, type, msg);
}

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

	SDL_mutex *lock;
	bool first_load;

	int32 access_time;
	void *lib;
};

static GameHandle game;

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

struct timespec _spec;
uint64 start_time;
uint64 last_time;
uint64 current_time;

#define SPEC_TO_SEC(s) ((s).tv_sec * 1000000000) + (s).tv_nsec
#define NSEC_TO_SEC (0.000000001)

float64 get_time()
{
	clock_gettime(CLOCK_MONOTONIC, &_spec);
	return (start_time - SPEC_TO_SEC(_spec)) * NSEC_TO_SEC;
}

bool load_libgame(GameHandle *handle)
{
	const char *path = "./bin/libbear.so";
	GameHandle game = *handle;

	int32 new_last_edit = get_file_edit_time(path);
	if (new_last_edit == game.access_time)
	{
		return false;
	}

	void *temp_handle = dlopen(path, RTLD_NOW);
	if (!temp_handle)
	{
		printf("[DL-ERROR] %s\n", dlerror());
		PRINT("Failed to load libbear.so!\n");
		return false;
	}

	dlerror();
	dlsym(temp_handle, "step");
	auto error_code = dlerror();
	if (error_code)
	{
		return false;
	}
	dlclose(temp_handle);

	SDL_LockMutex(game.lock);
	if (game.lib)
	{
		game.replace();
		dlclose(game.lib);
	}
	game.lib = dlopen(path, RTLD_NOW);

	game.step	= (StepFunc)  dlsym(game.lib, "step");
	if (!game.step)
	{
		PRINT("Failed to find step!\n");
		return false;
	}
	game.sound	= (SoundFunc) dlsym(game.lib, "sound");
	if (!game.sound)
	{
		PRINT("Failed to find sound!\n");
		return false;
	}
	game.reload = (ReloadFunc) dlsym(game.lib, "reload");
	if (!game.reload)
	{
		PRINT("Failed to find reload!\n");
		return false;
	}
	game.replace = (ReplaceFunc) dlsym(game.lib, "replace");
	if (!game.replace)
	{
		PRINT("Failed to find replace!\n");
		return false;
	}
	game.destroy = (DestroyFunc) dlsym(game.lib, "destroy");
	if (!game.destroy)
	{
		PRINT("Failed to find destroy!\n");
		return false;
	}

	if (game.first_load)
	{
		game.first_load = false;
		InitFunc init = (InitFunc) dlsym(game.lib, "init");
		if (!init)
		{
			PRINT("Failed to find init!\n");
			return false;
		}
		init(plt, &mem);
	}
	game.reload(plt, &mem);

	PRINT("Reload!");
	game.access_time = new_last_edit;
	*handle = game;
	SDL_UnlockMutex(game.lock);
	return true;
}

void plt_audio_callback(void *userdata, uint8 *stream, int32 length)
{
	SDL_LockMutex(game.lock);
	game.sound((int16 *) stream, length / (sizeof(float32) / sizeof(uint8)));
	SDL_UnlockMutex(game.lock);
}

int main(int varc, char *varv[])
{
	plt.log = linux_log;
	plt.print = printf;

	plt.read_file = read_entire_file;
	plt.last_write = get_file_edit_time;
	plt.get_time = get_time;

	plt.axis_value = axis_value;
	plt.button_state = button_state;

	mem.static_memory_size = GIGABYTE(1);
	mem.static_memory = (uint8 *) malloc(mem.static_memory_size);
	if (!mem.static_memory)
	{
		HALT_AND_CATCH_FIRE();
	}

	mem.temp_memory_size = GIGABYTE(1);
	mem.temp_memory = (uint8 *) malloc(mem.temp_memory_size);
	if (!mem.temp_memory)
	{
		HALT_AND_CATCH_FIRE();
	}

	if (SDL_Init(SDL_INIT_AUDIO | SDL_INIT_VIDEO | SDL_INIT_GAMECONTROLLER) != 0)
	{
		LOG("LOAD ERROR", "Unable to initalize SDL.");
		SDL_Quit();
		return(-1);
	}

	SDL_Window *window = SDL_CreateWindow(
			"Space Bears",
			SDL_WINDOWPOS_UNDEFINED,
			SDL_WINDOWPOS_UNDEFINED,
			WINDOW_WIDTH,
			WINDOW_HEIGHT,
			SDL_WINDOW_OPENGL
			);

	SDL_RaiseWindow(window);
	
	// TODO: Use OpenGL 3.3, or newer. This is just to get hello triangle.
	SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 2);
	SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 1);
	SDL_GL_SetAttribute(SDL_GL_DOUBLEBUFFER, 1);
	SDL_GL_SetAttribute(SDL_GL_DEPTH_SIZE, 24);
	SDL_GL_CreateContext(window);
	SDL_GL_SetSwapInterval(1);

	if (gladLoadGL() == 0)
	{
		LOG("OPENGL", "Unable to load OpenGL.");
		SDL_Quit();
		return(-1);
	}

	SDL_AudioSpec audio_spec = {};
	audio_spec.callback = plt_audio_callback;
	audio_spec.freq = spec_freq; // Is this dumb? Is 44100 better?
	audio_spec.format = AUDIO_S16; // Maybe too high rez?
	audio_spec.channels = 2; // This needs to be changeable.
	audio_spec.samples = 1024; // Ideally we want this as small as possible.
	auto audio_device = SDL_OpenAudioDevice(NULL, 0, &audio_spec, NULL, 0);
	if (!audio_device)
	{
		LOG("AUDIO", "Unable to load audio.");
		SDL_Quit();
		return(-1);
	}

	init_input();

	game.first_load = true;
	game.lock = SDL_CreateMutex();
	if (!load_libgame(&game))
	{
		LOG("LOAD ERROR", "Failed to load libgame.so");
		SDL_Quit();
		return(-1);
	}

	SDL_PauseAudioDevice(audio_device, 0);
	
	PRINT("Linux launch!");


	
	clock_gettime(CLOCK_MONOTONIC, &_spec);
	start_time = SPEC_TO_SEC(_spec);
	last_time = SPEC_TO_SEC(_spec);
	float32 delta = 0.0f; 

	bool running = true;
	while (running)
	{
		load_libgame(&game);
		
		update_input();

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
		clock_gettime(CLOCK_MONOTONIC, &_spec);
		current_time = SPEC_TO_SEC(_spec);
		delta = (float32) (current_time - last_time) * NSEC_TO_SEC;
		last_time = current_time;
	}

	destroy_input();
	game.destroy();
	SDL_DestroyMutex(game.lock);
	
	SDL_CloseAudio();
	SDL_Quit();

	return 0;
}
