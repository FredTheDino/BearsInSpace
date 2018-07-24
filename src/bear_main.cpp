// This file is compileda on each platform.
// This file should _NOT HAVE ANY_ platform specific code,
// that code should be placed on the platform layer.


#define BEAR_GAME
#include "bear_main.h"

// Misc
#include "bear_array.h"

// Math
#include "math/bear_math.h"

// GFX
#include "glad.c"
#include "bear_obj_loader.cpp"

#include "bear_gfx.h"
#define GL_LOADED glClear

// Audio
#include "audio/bear_audio.cpp"
#include "audio/bear_mixer.cpp"

// ECS
#include "ecs/bear_ecs.cpp"

// Physics
#include "physics/bear_physics.h"

// Tests
#include "bear_test.cpp"

GFX::Renderable renderable;
GFX::VertexBuffer vertex_buffer;
GFX::VertexArray vertex_array;
GFX::ShaderProgram program;
GFX::Texture texture;
Transform transform = create_transform();
Camera camera = create_camera(create_perspective_projection(PI / 4, ASPECT_RATIO, .01f, 100.0f));

#if 0
float32 rotx = 0;
float32 roty = 0;
#endif
float32 speed = 6.0f;

#if 1
void update(float32 delta)
{
#if 0
	// Temporary test code.
	EntityID entity = add_entity(&world->ecs);
	Position comp;
	comp.type = C_POSITION;
	comp.position = {1.0f, 2.0f, 3.0f};

	Blargh blarg;
	blarg.type = C_BLARGH;
	blarg.a = 32;
	blarg.b = 11111111;

	add_components(&world->ecs, entity, &comp, &blarg);

	EntityID entity2 = add_entity(&world->ecs);
	add_component(&world->ecs, entity2, C_BLARGH, (BaseComponent *) &blarg);

	remove_entity(&world->ecs, entity);
	remove_entity(&world->ecs, entity2);

	run_system(S_HELLO_WORLD, world, delta);

	id = load_sound(&world->audio, "res/smack.wav");
	stockhousen = load_sound(&world->audio, "res/stockhausen.wav");
	play_music(&world->audio, stockhousen, 1.0f, 1.0f, true);
	world->audio.left = {-1.0f, 0.0f, 0.0f};
	world->audio.position = { (float32) sin(t / 2.0f), 0.0f, 2.0f};
	t += delta;

	static bool pressed_jump = false;
	if (world->input.jump)
	{
		if (!pressed_jump)
		{
			pressed_jump = true;
			play_sound(&world->audio, id, 0.5f, 0.5f);
		}
	}
	else
	{
		pressed_jump = false;
	}
#endif

	float32 rx = AXIS_VAL("xrot") * 3.0f * delta;
	float32 ry = AXIS_VAL("yrot") * 3.0f * delta;

	if (rx || ry)
	{
		world->camera.rotx -= rx;
		world->camera.roty -= ry;
	}
	camera.transform.rot = toQ(world->camera.roty, world->camera.rotx, 0);

	float32 dx = AXIS_VAL("xmove") * speed * delta;
	float32 dz = AXIS_VAL("zmove") * speed * delta;
	if (dx || dz)
	{
		world->camera.position.x += dx * cos(-world->camera.rotx) - dz * sin(-world->camera.rotx);
		world->camera.position.z += dz * cos(-world->camera.rotx) + dx * sin(-world->camera.rotx);
	}

	world->camera.position.y += (AXIS_VAL("up") - AXIS_VAL("down")) * speed * delta;

	camera.transform.pos = world->camera.position;
}

void draw()
{
	glClearColor(0.1f, 0.1f, 0.1f, 1.0f);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	GFX::debug_draw_point({ .0f, .0f, .0f }, { .5f, .75f, .25f });

#if 0
	GFX::bind(texture);
	GFX::draw(renderable);

	GFX::debug_draw_line({ .0f, 1.0f, .0f }, { .0f, 2.0f, .0f }, { 1.0f, .0f, .0f });
	GFX::debug_draw_point({ .0f, 2.5f, .0f }, { .0f, 1.0f, .0f });
#endif
}
#endif

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
		
	}

	if (should_run_tests)
	{
		run_tests();
#if 1
		// Test code.
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
#endif
	}

	
	update(delta);
	draw();
	update_physics(NULL, 0.0f);
}


