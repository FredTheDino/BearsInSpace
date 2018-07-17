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
#include "bear_image_loader.cpp"
#include "bear_gfx.h"
#define GL_LOADED glClear

// Audio
#include "audio/bear_audio.cpp"
#include "audio/bear_mixer.cpp"

// ECS
#include "ecs/bear_ecs.cpp"

// Physics
#include "physics/physics.h"

// Tests
#include "bear_test.cpp"

GFX::Renderable renderable;
GFX::VertexBuffer vertex_buffer;
GFX::VertexArray vertex_array;
GFX::ShaderProgram program;
Transform transform = create_transform();
Camera camera = create_camera(create_perspective_projection(M_PI / 2, ASPECT_RATIO, .01f, 100.0f));

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

	transform.rot *= toQ(0, 0, 0.01f);

	if (B_PRESSED("jump"))
		transform.pos.y += .25f;
	else if (B_RELEASED("jump"))
		transform.pos.y -= .25f;
	transform.pos.x += AXIS_VAL("tiltx") * .03f;
	transform.pos.y -= AXIS_VAL("tilty") * .03f;
}

void draw()
{
	glClearColor(0.1f, 0.1f, 0.1f, 1.0f);
	glClear(GL_COLOR_BUFFER_BIT);
	
	GFX::draw(renderable);
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
	}

	update_physics(NULL, 0.0f);

	if (should_run_tests)
	{
		run_tests();
#if 1
		// Test code.
		Mesh mesh = load_mesh("res/monkey.obj");
		free_mesh(mesh);


		GFX::init_matrix_profiles();
		
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

	// Draw
	glClearColor(0.1f, 0.1f, 0.1f, 1.0f);
	glClear(GL_COLOR_BUFFER_BIT);

	
	update(delta);
	draw();
}


