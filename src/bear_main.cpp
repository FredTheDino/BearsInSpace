// Exists just here.

#define GL_LOADED glClear

#define BEAR_GAME
#include "bear_main.h"
#include "math/bear_math.h"
#include "bear_obj_loader.cpp"
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
GFX::Texture texture;
Transform transform = create_transform();
Camera camera = create_camera(create_perspective_projection(M_PI / 2, ASPECT_RATIO, .01f, 100.0f));

float32 rotx = 0;
float32 roty = 0;
float32 speed = .1f;

void update(float32 delta)
{

	if (should_run_tests)
	{
		run_tests();
	}

	float32 rx = AXIS_VAL("xrot") * .03f;
	float32 ry = AXIS_VAL("yrot") * .03f;

	if (rx || ry)
	{
		rotx -= rx;
		roty -= ry;
		camera.transform.rot = toQ(roty, rotx, 0);
	}
	float32 dx = AXIS_VAL("xmove") * speed;
	float32 dz = AXIS_VAL("zmove") * speed;
	if (dx || dz)
	{
		camera.transform.pos.x += dx * cos(-rotx) - dz * sin(-rotx);
		camera.transform.pos.z += dz * cos(-rotx) + dx * sin(-rotx);
	}

	camera.transform.pos.y += (AXIS_VAL("up") - AXIS_VAL("down")) * speed;
}

void draw()
{
	glClearColor(0.1f, 0.1f, 0.1f, 1.0f);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	GFX::bind(texture);
	GFX::draw(renderable);

	GFX::debug_draw_line({ .0f, 1.0f, .0f }, { .0f, 2.0f, .0f }, { 1.0f, .0f, .0f });
	GFX::debug_draw_point({ .0f, 2.5f, .0f }, { .0f, 1.0f, .0f });
}

extern "C"
void step(World *_world, float32 delta)
{
	world = _world;
	
	// Initialize GLAD if necessary
	if (!GL_LOADED)
	{
		gladLoadGL();

		glEnable(GL_DEPTH_TEST);

		GFX::init_matrix_profiles();

		GFX::init_debug();
		
		//TODO: REMOVE REST OF THIS IF-STATEMENT
		Mesh mesh = load_mesh("res/monkey.obj");
		free_mesh(mesh);

		// Shader program
		Array<GFX::ShaderInfo> shader_info = {
			{ GL_VERTEX_SHADER, "src/shader/simple.vert" },
			{ GL_FRAGMENT_SHADER, "src/shader/simple.frag" }
		};

		program = GFX::create_shader_program(shader_info);
		delete_array(&shader_info);

		texture = GFX::create_texture("res/test2.png");
		
		// Vertex buffer
		Array<float32> data_vb = {
			-.5f, .0f, .5f, .0f, .0f,
			-.5f, .0f, -.5f, 2.0f, .0f,
			.5f, .0f, -.5f, .0f, .0f,
			.5f, .0f, .5f, 2.0f, .0f,
			.0f, 1.0f, .0f, .5f, 2.0f
		};
		
		vertex_buffer = GFX::create_vertex_buffer(data_vb);
		delete_array(&data_vb);

		// Index buffer
		Array<uint32> data_ib = {
			0, 4, 3,
			1, 4, 0,
			2, 4, 1,
			3, 4, 2
		};

		GFX::IndexBuffer index_buffer = GFX::create_index_buffer(data_ib);
		
		delete_array(&data_ib);
		
		// Vertex array
		Array<GFX::VertexAttribute> attributes = {
			{ vertex_buffer, 0, 3, GL_FLOAT, false, 5 << 2, (void *) 0 },
			{ vertex_buffer, 1, 2, GL_FLOAT, false, 5 << 2, (void *) (3 << 2) }
		};
		vertex_array = GFX::create_vertex_array(attributes, index_buffer);
		delete_array(&attributes);

		// Renderable
		renderable.vertex_array = vertex_array;
		renderable.num_vertices = 12;
		renderable.program = program;
		
		// Model matrix
		renderable.matrix_profiles = create_array<GFX::MatrixProfile>(1);
		GFX::MatrixProfile transform_profile = {};
		transform_profile.uniform_name = "m_model";
		transform_profile.transform = &transform;
		
		append(&renderable.matrix_profiles, transform_profile);

		// View matrix
		GFX::add_matrix_profile("m_view", &camera);
		camera.transform.pos.z = 1;
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


