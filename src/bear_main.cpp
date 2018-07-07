// Exists just here.

#define GL_LOADED glClear

#define BEAR_GAME
#include "bear_main.h"
#include "bear_array.h"
#include "math/bear_math.h"
#include "bear_obj_loader.cpp"
#include "bear_image_loader.cpp"
#include "glad.c"

#include "bear_test.cpp"
#include "bear_audio.cpp"

// This file is included in each platform specific file. 
// This file should _NOT HAVE ANY_ platform specific code.

// TODO: This is temporary. We shouldn't rely on printf 
// since windows dosn't allow it when not running a console 
// application.



void update(float32 delta)
{

	if (should_run_tests)
	{
		run_tests();

		// TEMP!
		AudioID id = load_sound(&world->audio, "res/sine.wav");
		play_sound(&world->audio, id, 1.0f, 1.0f);
	}
}

void draw()
{
	// Initialize GLAD if necessary
	if (!GL_LOADED)
	{
		gladLoadGL();
#if 0
		Mesh mesh = load_mesh("res/monkey.obj");
		free_mesh(mesh);
#endif
	}

	glClearColor(0.1f, 0.1f, 0.1f, 1.0f);
	glClear(GL_COLOR_BUFFER_BIT);

	glBegin(GL_TRIANGLES);
	glColor3f(0.0f, 0.0f, 1.0f);
	glVertex2f(-0.5f, -0.5f);
	glColor3f(0.0f, 1.0f, 0.0f);
	glVertex2f(0.0f, 0.5f);
	glColor3f(1.0f, 0.0f, 0.0f);
	glVertex2f(0.5f, -0.5f);
	glEnd();
}

extern "C"
void step(World *_world, float32 delta)
{
	world = _world;
	update(delta);
	draw();
}

#define PI 3.1419f

//uint32 spec_freq = 44100;

extern "C"
void sound(float32 *out_buffer, int32 num_samples)
{
	// TODO(Ed): This is SLOW!!!! We will probably have to SIMD this.
	for (int32 i = 0; i < num_samples; i++)
		out_buffer[i] = 0.0f;

#if 1
	Audio *audio = &world->audio;
	for (uint32 i = 0; i < audio->max_source; i++)
	{
		AudioSource source = audio->sources[i];
		AudioBuffer buffer = get_buffer(audio, source.buffer_id);
		int32 samples_left = buffer.length - source.current_sample;
		uint32 length = num_samples < samples_left ? num_samples : samples_left;
		float32 *ptr = out_buffer;

		float32 *sampler = buffer.data32 + source.current_sample;
		audio->sources[i].current_sample += length;

		while (length)
		{
			float32 sample = *(sampler++);
			sample *= 2.0f;
			*(ptr++) += sample;
			*(ptr++) += sample;
			length -= 2;
		}
	}
#endif

#if 0
	static float32 t = 0;
	while (num_samples)
	{
		float32 sample = sin(t * 2 * 442 * PI);
		//sample = 0.0f; // This is so it doesn't get annoying.
		t += 1.0f / spec_freq;
		float32 left_panning = sin(t * 2 * PI * 0.1f) * 0.5f + 0.5f;
		*out_buffer++ = sample * left_panning;
		*out_buffer++ = sample * (1.0f - left_panning);
		num_samples -= 2;
	}
#endif
}


