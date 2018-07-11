void init_ecs(World *world)
{
	PLT *plt = &world->plt;
	ECS *ecs = &world->ecs;
	const int32 inital_size = 50;

	ecs->entities = MALLOC2(Entity, inital_size);
	ASSERT(ecs->entities); // TODO(Ed): We assume we manage to get some memory.
	ecs->uid_counter = 0;
	ecs->free_entity = 0;
	ecs->max_entity = 0;
	ecs->allocated_entities = inital_size;

	ECSEntry *cs = ecs->component_types;
	for (uint32 i = 0; i < NUM_COMPONENTS; i++)
	{
		cs[i] = {};
	}

#define COMP_ENTRY(id, type)\
	cs[id] = { sizeof(type), inital_size, 0, plt->malloc("ECS_INIT", id, sizeof(type) * inital_size) };

	// Entries go here. (Order doesn't matter)
	COMP_ENTRY(C_POSITION, Position);
	COMP_ENTRY(C_BLARGH, Blargh);

	// Entries end here.

	for (uint32 i = 0; i < NUM_COMPONENTS; i++)
	{
		if (cs[i].c)
			continue;
		LOG("ECS", "No Initalization for component type %u. Please check 'init_ecs' in '" __FILE__ "'\n");
	}
};

void destroy_ecs(World *world)
{
	PLT *plt = &world->plt;
	ECS *ecs = &world->ecs;
	ECSEntry *cs = ecs->component_types;
	for (uint32 i = 0; i < NUM_COMPONENTS; i++)
	{
		if (cs[i].c)
			plt->free(cs[i].c);
	}

	plt->free(ecs->entities);
}

void s_hello_world(World *world, float32 delta)
{
	world->plt.print("Delta is %.2f\n", delta);
}

void run_system(SystemType type_id, World *world, float32 delta)
{
	ASSERT(type_id < NUM_SYSTEMS);
	switch (type_id)
	{
#define SYSTEM_ENTRY(id, func_name) case id: func_name(world, delta); break;

		SYSTEM_ENTRY(S_HELLO_WORLD, s_hello_world);

	default:
		ASSERT(!"Unknown systemtype!");
	}
}

