// Exists just here.

#define GL_LOADED glClear

#define BEAR_GAME
#include "bear_main.h"
#include "bear_array.h"
#include "math/bear_math.h"
#include "bear_obj_loader.cpp"
#include "bear_image_loader.cpp"
#include "glad.c"

#include "bear_gfx.h"

#include "bear_test.cpp"

// This file is included in each platform specific file. 
// This file should _NOT HAVE ANY_ platform specific code.

// TODO: This is temporary. We shouldn't rely on printf 
// since windows dosn't allow it when not running a console 
// application.

GFX::Renderable renderable;
GFX::VertexBuffer vertex_buffer;
GFX::VertexArray vertex_array;
GFX::ShaderProgram program;
Transform transform = create_transform();
Camera camera;

void update(float32 delta)
{

	if (should_run_tests)
	{
		run_tests();
	}

	
}

void draw()
{
	glClearColor(0.1f, 0.1f, 0.1f, 1.0f);
	glClear(GL_COLOR_BUFFER_BIT);

	printf("%f\n", transform.pos.x);
	
	GFX::draw(renderable);
}

extern "C"
void step(World *_world, float32 delta)
{
	world = _world;

	// Initialize GLAD if necessary
	if (!GL_LOADED)
	{
		gladLoadGL();
		Mesh mesh = load_mesh("res/monkey.obj");
		free_mesh(mesh);

		// Shader program
		Array<GFX::ShaderInfo> shader_info = {
			{ GL_VERTEX_SHADER, "src/shader/simple.vert" },
			{ GL_FRAGMENT_SHADER, "src/shader/simple.frag" }
		};

		program = GFX::create_shader_program(shader_info);
		delete_array(&shader_info);
		
		// Vertex buffer
		Array<float32> data = {
			.0f, .5f,
			.5f, -.5f,
			-.5f, -.5f
		};
		
		vertex_buffer = GFX::create_vertex_buffer(data);
		delete_array(&data);
		
		// Vertex array
		Array<GFX::VertexAttribute> attributes = { { vertex_buffer, 0, 2, GL_FLOAT } };
		vertex_array = GFX::create_vertex_array(attributes);
		delete_array(&attributes);

		// Renderable
		renderable.vertex_array = vertex_array;
		renderable.num_vertices = 3;
		renderable.program = program;
		
		// Matrices
		renderable.matrix_profiles = create_array<GFX::MatrixProfile>(1);
		GFX::MatrixProfile transform_profile = {};
		transform_profile.uniform_name = "m_model";
		transform_profile.transform = &transform;
		append(&renderable.matrix_profiles, transform_profile);
	}
	
	update(delta);
	draw();
}

#define PI 3.1419f

uint32 spec_freq = 44100;
float32 t = 0;

extern "C"
void sound(float32 *buffer, int32 num_samples)
{
	while (num_samples)
	{
		float32 sample = sin(t * 2 * 442 * PI);
		sample = 0.0f; // This is so it doesn't get annoying.
		t += 1.0f / spec_freq;
		float32 left_panning = sin(t * 2 * PI * 0.1f) * 0.5f;
		*buffer++ = sample * left_panning;
		*buffer++ = sample * (1.0f - left_panning);
		num_samples -= 2;
	}
}


