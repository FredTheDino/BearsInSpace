
void generate_astroid_field(World *world, uint32 num_asteroids, Vec3f min, Vec3f max)
{
	RandomState rng = seed(131236);
	CTransform t;
	t.type = C_TRANSFORM;

	CBody body = {};
	body.type = C_BODY;
	body.inverse_mass = 0.1f;
	body.linear_damping = 0.90f;
	body.angular_damping = 0.90f;
	body.shape = make_box(1.0f, 1.0f, 1.0f);
	body.inverse_inertia = inverse(calculate_inertia_tensor(body.shape, 10.0f));

	CAsteroid ast = {};
	ast.type = C_ASTEROID;

	for (uint32 i = 0; i < num_asteroids; i++)
	{
		float32 min_speed = 0.1f;
		float32 max_speed = 0.3f;
		float32 rand_speed = random_float_in_range(&rng, min_speed, max_speed);
		
		float32 min_scale = 0.2f;
		float32 max_scale = 2.2f;
		t.scale = random_vec3f(&rng, V3(1.0f, 1.0f, 1.0f) * min_scale, V3(1.0f, 1.0f, 1.0f) * max_scale);
		Vec3f angle = random_vec3f(&rng) * PI;
		t.orientation = toQ(angle.x, angle.y, angle.z);
		t.position = random_vec3f(&rng, min, max);
		body.velocity = -t.position * rand_speed;
		add_components(&world->ecs, &world->phy, add_entity(&world->ecs), &body, &t, &ast);
	}
}

void draw_asteroids(World *world)
{
	ECSEntry entry = get_all_components(&world->ecs, C_ASTEROID);
	AssetID asset_id = get_asset_id(BAT_MESH, "default", 0);

	GFX::Renderable renderable = {}; 
	renderable.vertex_array = get_asset(asset_id).vao;
	renderable.num_vertices = get_asset(asset_id).draw_length;
	renderable.program = program;

	GFX::MatrixProfile transform_profile = {};
	transform_profile.uniform_name = "m_model";
	for (uint32 i = 0; i < entry.length; i++)
	{
		CAsteroid ast = ((CAsteroid *) entry.c)[i];
		CTransform t = *((CTransform *) get_component(&world->ecs, ast.base.owner, C_TRANSFORM));

		transform_profile.transform = &t.transform;
		renderable.matrix_profiles = temp_array<GFX::MatrixProfile>(1);
		append(&renderable.matrix_profiles, transform_profile);

		GFX::bind(default_image.texture);
		GFX::draw(renderable);
	}
}

