// This file is compileda on each platform.
// This file should _NOT HAVE ANY_ platform specific code,
// that code should be placed on the platform layer.

struct GameMemory;
GameMemory *mem;

#include "bear_shared.h"
#include "bear_memory.cpp"

PLT plt;

#include "bear_main.h"
World *world;

// Audio
#include "audio/bear_audio.cpp"
#include "audio/bear_mixer.cpp"

// Utility
//#include "bear_utility.h"

// GFX
#include "glad.c"
#define GL_LOADED glClear
#include "bear_obj_loader.cpp"
#include "bear_gfx.h"

// Font
#include "gfx/bear_font.h"

// ECS
#include "ecs/bear_ecs.cpp"

// Physics
#include "physics/bear_physics.cpp"

// Clocks
#include "bear_clock.cpp"

// Draw function for ECS (should preferably be the last include)
#include "gfx/bear_draw_ecs.h"

#if 0
// Tests
#include "bear_test.cpp"
#endif

#if 1
GFX::VertexBuffer vertex_buffer;
GFX::VertexArray vertex_array;
GFX::ShaderProgram program;
GFX::Texture texture;
Transform transform = create_transform();
Camera camera;
#endif

#if 0
float32 rotx = 0;
float32 roty = 0;

Mesh cone;
float32 speed = 6.0f;
#endif


AudioID buffer;

// Enter APP
extern "C"
void init(PLT _plt, GameMemory *_mem)
{
	mem = _mem;
	plt = _plt;
	world = static_push_struct(World); // This must allways be the first allocation.

	if (!GL_LOADED)
	{
		auto error = gladLoadGL();
		ASSERT(error);
		glEnable(GL_DEPTH_TEST);
		glEnable(GL_BLEND);
		glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
		
		GFX::init_matrix_profiles();
		
	}
	
	init_ecs(&world->ecs);
	init_phy(&world->phy);

	buffer = load_sound(&world->audio, "res/stockhausen.wav");

	camera = create_camera(create_perspective_projection(PI / 4, ASPECT_RATIO, .01f, 100.0f));
	//camera.transform.position = {-30.0f, 25.0f, 20.0f};
	//camera.transform.orientation = toQ(5.7f, -1.0f, 0);
	GFX::add_matrix_profile("m_view", &camera);
	
	// Font
	load_font("open-sans", "res/fonts/open-sans/OpenSans-Regular.ttf");
	
	// Output graphics
	world->output_buffer = GFX::create_frame_buffer(temp_array<uint32>({ GL_COLOR_ATTACHMENT0 }), WINDOW_WIDTH, WINDOW_HEIGHT);
	world->output_vb = GFX::create_vertex_buffer(temp_array<float32>({ -1, -1, 1, -1, -1, 1, 1, -1, 1, 1, -1, 1 }));
	world->output_quad = GFX::create_vertex_array(
		temp_array<GFX::VertexAttribute>({
				{ world->output_vb, 0, 2, GL_FLOAT }
		}));
	world->output_program = GFX::create_shader_program(temp_array<GFX::ShaderInfo>({ { GL_VERTEX_SHADER, "src/shader/post.vert" }, { GL_FRAGMENT_SHADER, "src/shader/post.frag" } }));
}

// Reload the library.
extern "C"
void reload(PLT _plt, GameMemory *_mem)
{
	if (!world)
	{
		mem = _mem;
		plt = _plt;
	}
	
	if (!GL_LOADED)
	{
		gladLoadGL();
	}
	
	GFX::init_debug();
	PRINT("hej");
	GFX::init_font_rendering();
	PRINT("då\n");
	//play_sound(&world->audio, buffer, 1.0f, 1.0f);
	// How the fk does the graphics work?

	clear_ecs(&world->ecs, &world->phy);

	CTransform transform = {};
	transform.type = C_TRANSFORM;
	transform.position = {0.0f, 5.0f, -4.0f};
	transform.scale = {1.0f, 1.0f, 1.0f};
	transform.orientation = toQ(1.5f, 1.5f, 0.0f);

	CBody body = {};
	body.type = C_BODY;
	body.inverse_mass = 0.1f;
	body.velocity = {0.0f, -1.0f, 1.0f};
	body.rotation = {0.0f, 1.0f, 1.0f};
	body.linear_damping = 0.80f;
	body.angular_damping = 0.80f;
	body.shape = make_box(2.0f, 2.0f, 2.0f);
	body.inverse_inertia = inverse(calculate_inertia_tensor(body.shape, 10.0f));

	EntityID e = add_entity(&world->ecs);
	add_components(&world->ecs, &world->phy, e, &body, &transform);

	transform.type = C_TRANSFORM;
	transform.position = {-1.0f, 1.0f, 1.0f};
	transform.orientation = {1.0f, 0.0f, 0.0f, -0.05f};

	body.type = C_BODY;
	body.inverse_mass = 0.0f;
	body.velocity = {0.0f, 0.0f, 0.0f};
	body.rotation = {0.0f, 0.0f, 0.0f};
	body.linear_damping = 0.0f;
	body.angular_damping = 0.0f;
	body.shape = make_box(10.0f, 1.0f, 10.0f);
	body.inverse_inertia = inverse(calculate_inertia_tensor(body.shape, 0.0f));

	EntityID f = add_entity(&world->ecs);
	add_components(&world->ecs, &world->phy, f, &body, &transform);
}

// Exit APP
extern "C"
void destroy()
{
	PRINT("Destroy!\n");
	GFX::delete_shader_program(world->output_program);
	GFX::delete_vertex_array(world->output_quad);
	GFX::delete_vertex_buffer(world->output_vb);
	GFX::delete_frame_buffer(world->output_buffer);
}

// Exit the library
extern "C"
void replace()
{
	PRINT("Replace!\n");
	GFX::destroy_debug();
	GFX::destroy_font_rendering();
	PRINT("WHAT\n");
}


extern "C"
void step(float32 delta)
{
	//LOG("DEBUG", "MEEEEEEEEEEEEEEEEEEEEP!");
	reset_debug_clock();


	auto phy_clock = start_debug_clock("Physics Step");
	run_system(S_PHYSICS, world, minimum(delta, 1.0f / 30.0f));
	stop_debug_clock(phy_clock);

	phy_clock = start_debug_clock("Physics Draw");

	/* -- START MOVEMENT -- */
	float32 planar_speed = 15.0f * delta;
	float32 vertical_speed = 15.0f * delta;
	float32 rotational_speed = 1.5f * delta;
	Vec3f movement = {};
	movement.x = AXIS_VAL("xmove") * planar_speed;
	movement.z = AXIS_VAL("zmove") * planar_speed;
	movement = camera.transform.orientation * movement;
	movement.y += (AXIS_VAL("up") - AXIS_VAL("down")) * vertical_speed;
	camera.transform.position += movement;
	camera.transform.orientation = 
		toQ(0.0f, -AXIS_VAL("xrot") * rotational_speed, 0.0f) *
		camera.transform.orientation *
		toQ(-AXIS_VAL("yrot") * rotational_speed, 0.0f, 0.0f);
	/* -- END MOVEMENT -- */

	stop_debug_clock(phy_clock);
	
	//display_clocks();

	GFX::draw(world->output_buffer, &world->ecs, false);
	GFX::draw_to_screen();
}

#if 0
	// Step
	if (should_run_tests)
	{
		{

			{
				Transform t;
				t.scale = {2.0f, 3.0f, 1.0f,};
				t.orientation = toQ({1.0f, 2.0f, 3.0f}, PI * 0.33f);
				t.position = {1.0f, 1.0f, -1.0f};
				Vec3f p = {1.0f, 1.0f, 1.0f};
				Vec3f t_p = t * p;
				Vec3f rt_p = t / t_p;
				ASSERT(p == rt_p);

				Mat4f m = {
					1.0f, 0.0f, 0.0f, 0.0f,
					1.0f, 1.0f, 0.0f, 0.0f,
					1.0f, 0.0f, 1.0f, 0.0f,
					0.0f, 0.0f, 0.0f, 1.0f
				};
				p = {1.0f, 0.0f, 0.0f};
				Vec3f n = m * p;
				n = n;
			}
			camera = create_camera(create_perspective_projection(PI / 4, ASPECT_RATIO, .01f, 100.0f));
			

			clear_ecs(world);

			CTransform transform = {};
			transform.type = C_TRANSFORM;
			transform.position = {0.0f, 5.0f, -4.0f};
			transform.scale = {1.0f, 1.0f, 1.0f};
			transform.orientation = toQ(1.5f, 1.5f, 0.0f);

			CBody body = {};
			body.type = C_BODY;
			body.inverse_mass = 0.1f;
			body.velocity = {0.0f, -1.0f, 1.0f};
			body.rotation = {0.0f, 1.0f, 1.0f};
			body.linear_damping = 0.80f;
			body.angular_damping = 0.80f;
			body.shape = make_box(2.0f, 2.0f, 2.0f);
			body.inverse_inertia = inverse(calculate_inertia_tensor(body.shape, 10.0f));

#if 1 // Box
			e = add_entity(&world->ecs);
			add_components(&world->ecs, &world->phy, e, body, transform);
#endif

#if 1 // Plane
			transform.type = C_TRANSFORM;
			transform.position = {-1.0f, 1.0f, 1.0f};
			transform.orientation = {1.0f, 0.0f, 0.0f, -0.05f};

			body.type = C_BODY;
			body.inverse_mass = 0.0f;
			body.velocity = {0.0f, 0.0f, 0.0f};
			body.rotation = {0.0f, 0.0f, 0.0f};
			body.linear_damping = 0.0f;
			body.angular_damping = 0.0f;
			body.shape = make_box(10.0f, 1.0f, 10.0f);
			body.inverse_inertia = inverse(calculate_inertia_tensor(body.shape, 0.0f));

			EntityID f = add_entity(&world->ecs);
			add_components(&world->ecs, &world->phy, f, body, transform);
#endif
#if 0 // Cannon ball

			transform.type = C_TRANSFORM;
			transform.position = {-0.0f, 3.0f, -0.0f};

			cone = load_mesh("res/monkey.obj");
			if (cone.valid)

			body.type = C_BODY;
			body.inverse_mass = 1.0f;
			body.velocity = {0.0f, 0.0f, 0.0f};
			body.rotation = {0.0f, 0.0f, 2.0f};
			body.linear_damping = 1.0f;
			body.angular_damping = 0.99f;
			body.shape = make_mesh(cone.positions, cone.stride, cone.indices);
			body.inverse_inertia = inverse(calculate_inertia_tensor(body.shape, 1.0f));

			EntityID g = add_entity(&world->ecs);
			add_components(&world->ecs, &world->phy, g, body, transform);
#endif
#if 0

			transform.type = C_TRANSFORM;
			transform.pos = {0.0f, 0.0f, 0.0f};

			body.type = C_BODY;
			body.mass = 0.0f;
			body.velocity = {0.0f, 0.0f, 0.0f};
			body.shape = make_box(15.0f, 3.0f, 50.0f);

			EntityID h = add_entity(&world->ecs);
			add_components(&world->ecs, &world->phy, h, body, transform);
#endif
		}

		run_tests();

#if 1
		// Test code.
		Mesh mesh = load_mesh("res/monkey.obj");
		free_mesh(mesh);
		
		// Shader program
		Array<GFX::ShaderInfo> shader_info = {
			{ GL_VERTEX_SHADER, "src/shader/obj.vert" },
			{ GL_FRAGMENT_SHADER, "src/shader/obj.frag" }
		};

		program = GFX::create_shader_program(shader_info);
		delete_array(&shader_info);
		
		// Vertex buffer
		Mesh mesh = load_mesh("res/monkey.obj");
		Array<float32> data_vb = to_float32(mesh.positions);
		
		vertex_buffer = GFX::create_vertex_buffer(data_vb);
		delete_array(&data_vb);

		// Index buffer
		GFX::IndexBuffer index_buffer = GFX::create_index_buffer(mesh.indices);
		
		// Vertex array
		Array<GFX::VertexAttribute> attributes = {
			{ vertex_buffer, 0, 3, GL_FLOAT },
		};
		vertex_array = GFX::create_vertex_array(attributes, index_buffer);
		delete_array(&attributes);

		// Renderable
		renderable.vertex_array = vertex_array;
		renderable.num_vertices = size(mesh.indices);
		renderable.program = program;
		free_mesh(mesh);
		
		// Model matrix
		renderable.matrix_profiles = create_array<GFX::MatrixProfile>(1);
		GFX::MatrixProfile transform_profile = {};
		transform_profile.uniform_name = "m_model";
		transform_profile.transform = &transform;
		
		append(&renderable.matrix_profiles, transform_profile);

		// View matrix
		GFX::add_matrix_profile("m_view", &camera);
		camera.transform.position.z = 1;

		run_system(S_PHYSICS, world, minimum(delta, 1.0f / 30.0f)); 
#endif
	}
	
#if 0
	update(delta);
	draw();
	if (B_DOWN("forward"))
	{
		CBody *body = (CBody *) get_component(&world->ecs, e, C_BODY);
		Vec3f offset = {-0.0f, -1.0f, 0.0f};
		Vec3f impulse = {0.0f, 1.0f, 0.0f};
		impulse *= delta * 25.0f;
		relative_impulse(body, impulse, offset);
	}

	if (B_DOWN("left"))
	{
		CBody *body = (CBody *) get_component(&world->ecs, e, C_BODY);
		Vec3f offset = {-0.0f, -0.0f, -1.0f};
		Vec3f impulse = {0.0f, 1.0f, 0.0f};
		impulse *= delta * 25.0f;
		relative_impulse(body, impulse, offset);
	}

	if (B_DOWN("right"))
	{
		CBody *body = (CBody *) get_component(&world->ecs, e, C_BODY);
		Vec3f offset = {-0.0f, -0.0f, 1.0f};
		Vec3f impulse = {0.0f, 1.0f, 0.0f};
		impulse *= delta * 25.0f;
		relative_impulse(body, impulse, offset);
	}
#endif
#endif
