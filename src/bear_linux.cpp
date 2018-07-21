#include <SDL2/SDL.h>
#include <sys/stat.h>
#include <dlfcn.h>
#include <unistd.h>
#include <time.h>

#include "bear_main.h"
#include "glad.c"

typedef void (*StepFunc)(World *, float32);
typedef void (*SoundFunc)(float32 *, int32);

struct GameHandle
{
	SDL_mutex *lock;
	StepFunc  step;
	SoundFunc sound;

	int32 access_time;
	void *lib;
};

static GameHandle game;

// Dirty, I know.
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
		DEBUG_LOG("Failed to load libbear.so!");
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
		dlclose(game.lib);
	}
	game.lib = dlopen(path, RTLD_NOW);

	StepFunc  step	= (StepFunc)  dlsym(game.lib, "step");
	SoundFunc sound	= (SoundFunc) dlsym(game.lib, "sound");

	if (!step)
	{
		return false;
		DEBUG_LOG("Failed to load step function.");
	}
	game.step = step;
	if (!sound)
	{
		return false;
		DEBUG_LOG("Failed to load sound function.");
	}
	game.sound = sound;

	DEBUG_LOG("Reload!");
	game.access_time = new_last_edit;
	*handle = game;
	SDL_UnlockMutex(game.lock);
	return true;
}

void plt_audio_callback(void *userdata, uint8 *stream, int32 length)
{
	SDL_LockMutex(game.lock);
	game.sound((float32 *) stream, length / (sizeof(float32) / sizeof(uint8)));
	SDL_UnlockMutex(game.lock);
}

int main(int varc, char *varv[])
{
	world.plt.malloc = malloc_;
	world.plt.free = free_;
	world.plt.realloc = realloc_;

	world.plt.log = debug_log_;
	world.plt.print = printf;

	world.plt.read_file = read_entire_file;
	world.plt.free_file = free_file;
	world.plt.last_write = get_file_edit_time;

	world.plt.axis_value = axis_value;
	world.plt.button_state = button_state;

	world.__mem = (MemoryAllocation *)(void *)__mem;

	if (!load_libgame(&game))
	{
		DEBUG_LOG("Failed to load libgame.so");
		SDL_Quit();
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
	SDL_GL_SetSwapInterval(0);

	if (gladLoadGL() == 0)
	{
		DEBUG_LOG("Unable to load OpenGL.");
		SDL_Quit();
		return(-1);
	}

	SDL_AudioSpec audio_spec = {};
	audio_spec.callback = plt_audio_callback;
	audio_spec.freq = spec_freq; // Is this dumb? Is 44100 better?
	audio_spec.format = AUDIO_F32; // Maybe too high rez?
	audio_spec.channels = 2; // This needs to be changeable.
	audio_spec.samples = 2048; // Ideally we want this as small as possible.
	auto audio_device = SDL_OpenAudioDevice(NULL, 0, &audio_spec, NULL, 0);
	if (!audio_device)
	{
		DEBUG_LOG("Unable to load audio.");
		SDL_Quit();
		return(-1);
	}

	init_ecs(&world);

	SDL_PauseAudioDevice(audio_device, 0);
	
	init_input();
	
	DEBUG_LOG("Linux launch!");

#define SPEC_TO_SEC(s) ((s).tv_sec * 1000000000) + (s).tv_nsec
#define NSEC_TO_SEC (0.000000001f)

	struct timespec _spec;
	uint64 start_time;
	uint64 last_time;
	uint64 current_time;
	
	clock_gettime(CLOCK_MONOTONIC, &_spec);
	start_time = SPEC_TO_SEC(_spec);
	last_time = SPEC_TO_SEC(_spec);

	world.running = true;
	while (world.running)
	{
		load_libgame(&game);
		
		update_input();

		SDL_Event event;
		while (SDL_PollEvent(&event))
		{
			if (event.type == SDL_QUIT)
			{
				world.running = false;
			}
			else
			{
				handle_input_event(event);
			}
		}
		
		game.step(&world, world.clk.delta);
		SDL_GL_SwapWindow(window);

		// Timing
		clock_gettime(CLOCK_MONOTONIC, &_spec);
		current_time = SPEC_TO_SEC(_spec);
		world.clk.time  = (float64) (current_time - start_time) * NSEC_TO_SEC;
		world.clk.delta = (float32) (current_time - last_time) * NSEC_TO_SEC;
		last_time = current_time;
	}

	destroy_input();
	
	SDL_CloseAudio();
	SDL_Quit();

	destroy_ecs(&world);

	check_for_leaks();
	return 0;
}
