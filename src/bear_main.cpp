// This file is compileda on each platform.
// This file should _NOT HAVE ANY_ platform specific code,
// that code should be placed on the platform layer.

struct GameMemory;
GameMemory *mem;

#include "bear_shared.h"
PLT plt;
#include "bear_memory.cpp"

#include "bear_main.h"
World *world;

// Clocks
#include "bear_clock.cpp"

// Audio
#include "audio/bear_audio.cpp"
#include "audio/bear_mixer.cpp"

// GFX
#include "glad.c"
#define GL_LOADED glClear
#include "bear_gfx.h"

// ECS
#include "ecs/bear_ecs.cpp"

// Physics
#include "physics/bear_physics.cpp"


// Assets
#include "bear_loader.cpp"

// Random! :P
#include "bear_random.h"

GFX::ShaderProgram program;
#include "world/bear_world_gen.cpp"

#if 0
// Tests
#include "bear_test.cpp"
#endif


#include "glad.c"
#define GL_LOADED glClear

#if 1
Camera camera;
#endif

#if 0
float32 rotx = 0;
float32 roty = 0;

Mesh cone;
float32 speed = 6.0f;
#endif


AudioID buffer;

#if 0
inline
void setup_pointers()
{
	world = (World *) ((MemoryAllocation *) mem->static_memory + 1);
	world_audio = &world->audio;
	world_ecs = &world->ecs;
	world_phy = &world->phy;
	world_matrix_profiles = &world->matrix_profiles;
}
#endif

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
	}

	init_ecs(&world->ecs);
	init_phy(&world->phy);

	//buffer = load_sound(&world->audio, "res/stockhausen.wav");


	// TODO: This is ugly as fuck.
	//world->matrix_profiles = GFX::matrix_profiles;
}

RandomState rng;
// Reload the library.
extern "C"
void reload(PLT _plt, GameMemory *_mem)
{
	if (!world)
	{
		mem = _mem;
		plt = _plt;
		world = (World *) ((MemoryAllocation *) mem->static_memory + 1);
	}

	if (!GL_LOADED)
	{
		gladLoadGL();
		glEnable(GL_DEPTH_TEST);
	}

	start_loader();
	//load_asset(0);

	rng = seed(34565432654323);

	GFX::init_matrix_profiles();
	GFX::init_debug();
	camera = create_camera(create_perspective_projection(PI / 4, ASPECT_RATIO, .01f, 100.0f));
	camera.transform.position = {-30.0f, 25.0f, 20.0f};
	camera.transform.position *= 0.75f;
	camera.transform.orientation = toQ(5.7f, -1.0f, 0);
	GFX::add_matrix_profile("m_view", &camera);
	
	Array<GFX::ShaderInfo> shader_info = temp_array<GFX::ShaderInfo>({
		{ GL_VERTEX_SHADER, "src/shader/simple.vert" },
		{ GL_FRAGMENT_SHADER, "src/shader/simple.frag" }
	});

	program = GFX::create_shader_program(shader_info);


	//play_sound(&world->audio, buffer, 1.0f, 1.0f);
	// How the fk does the graphics work?

	clear_ecs(&world->ecs, &world->phy);

	// TODO: Realloc doens't work... I don't know why.
	float32 range = 50.0f;
	generate_astroid_field(world, 1000, V3(-1.0, -1.0f, -1.0f) * range, V3(1.0f, 1.0f, 1.0f) * range);

#if 0 
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
#endif
}

// Exit APP
extern "C"
void destroy()
{
}

// Exit the library
extern "C"
void replace()
{
	PRINT("MEM watermark: %d\n", get_static_memory_watermark());
	stop_loader();
}


extern "C"
void step(float32 delta)
{
	//LOG("DEBUG", "MEEEEEEEEEEEEEEEEEEEEP!");
	update_assets();
	reset_debug_clock();

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

	auto phy_clock = start_debug_clock("Physics");
	run_system(S_PHYSICS, world, minimum(delta, 1.0f / 30.0f)); 
	stop_debug_clock(phy_clock);


	auto draw_clock = start_debug_clock("Render");
	glClearColor(0.1f, 0.1f, 0.1f, 1.0f);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	debug_draw_engine(&world->ecs, &world->phy);
	draw_asteroids(world);
	stop_debug_clock(draw_clock);

	display_clocks();

#if 0
	GFX::Renderable renderable = {}; 
	renderable.matrix_profiles = temp_array<GFX::MatrixProfile>(1);
	AssetID asset_id = get_asset_id(BAT_MESH, "default", "mesh");
	renderable.vertex_array = get_asset(asset_id).vao;
	renderable.num_vertices = get_asset(asset_id).draw_length;
	renderable.program = program;

	Transform transform = create_transform();
	static float32 t = 0.0f;
	t += delta;
	transform.orientation = toQ(t * 0.1f, t * 0.5f, 0);
	transform.scale = V3(0.5f, 1.0f, 0.75f);
	transform.position.y = 3.0f;

	GFX::MatrixProfile transform_profile = {};
	transform_profile.uniform_name = "m_model";
	transform_profile.transform = &transform;

	append(&renderable.matrix_profiles, transform_profile);

	GFX::bind(default_image.texture);
	GFX::draw(renderable);

#endif
#if 0

	for (uint32 i = 0; i < 500; i++)
	{
		Vec2f p2 = random_unit_vec2f(&rng);
		Vec3f p = {p2.x, p2.y, 0.0f};
		GFX::debug_draw_point(p * 5.0f, {0.75f, 0.25f, 0.5f});
	}

#endif
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
			body.shape = make_mesh(cone.positions, cone.stride, cone.indicies);
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
#if 0
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

#if 0
// Old update.
void update(float32 delta)
{
	float32 rx = AXIS_VAL("xrot") * 3.0f * delta;
	float32 ry = AXIS_VAL("yrot") * 3.0f * delta;

	if (rx || ry)
	{
		world->camera.rotx -= rx;
		world->camera.roty -= ry;
	}
	camera.transform.orientation = toQ(world->camera.roty, world->camera.rotx, 0);

	float32 dx = AXIS_VAL("xmove") * speed * delta;
	float32 dz = AXIS_VAL("zmove") * speed * delta;
	if (dx || dz)
	{
		world->camera.position.x += dx * cos(-world->camera.rotx) - dz * sin(-world->camera.rotx);
		world->camera.position.z += dz * cos(-world->camera.rotx) + dx * sin(-world->camera.rotx);
	}

	world->camera.position.y += (AXIS_VAL("up") - AXIS_VAL("down")) * speed * delta;

	camera.transform.position = world->camera.position;
}

void draw()
{
	glClearColor(0.1f, 0.1f, 0.1f, 1.0f);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

#if 0
	GFX::debug_draw_point({ .0f, .0f, .0f }, { .5f, .75f, .25f });

	GFX::bind(texture);
	GFX::draw(renderable);

	GFX::debug_draw_line({ .0f, 1.0f, .0f }, { .0f, 2.0f, .0f }, { 1.0f, .0f, .0f });
	GFX::debug_draw_point({ .0f, 2.5f, .0f }, { .0f, 1.0f, .0f });
#endif
}
#endif
