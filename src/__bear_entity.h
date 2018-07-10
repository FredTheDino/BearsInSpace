#pragma once

//
// Meta info
//
enum SystemType
{
	// Types
	S_HELLO_WORLD,

	// Length
	NUM_SYSTEMS,
};

enum ComponentType
{
	// Types
	C_POSITION,
	C_BLARGH,

	// Length
	NUM_COMPONENTS,
};

struct EntityID
{
	int32 pos;
	int32 uid;
};

struct BaseComponent
{
	EntityID owner;
	ComponentType type;
};

struct Entity
{
	EntityID id;
	void *components[NUM_COMPONENTS];
};

struct ECSEntry
{
	int32 size;
	uint32 max;
	uint32 num;
	void *c; 
}

struct ECS
{
	ECSEntry component_types[NUM_COMPONENTS];

	uint32 max_entities;
	uint32 num_entities;
	Entity *entities;
};

void init_ecs(ECS *ecs, PLT *plt)
{
	const int32 inital_size = 50;
	ECSEntry *cs = ecs->component_types;

	for (uint32 i = 0; i < NUM_COMPONENTS; i++)
	{
		cs[i]->c = NULL;
	}

#define COMP_ENTRY(ID, type)\
	cs[ID] = { sizeof(type), inital_size, 0, plt->malloc("ECS_INIT", 0, sizeof(type) * inital_size) };

	// Entries go here. (Order doesn't matter)
	COMP_ENTRY(C_POSITION, Position);
	COMP_ENTRY(C_BLARGH, Blargh);

	// Entries end here.

	for (uint32 i = 0; i < NUM_COMPONENTS; i++)
	{
		if (cs[i]->c)
			continue;
		LOG("ECS", "No Initalization for component type %u. Please check 'init_ecs'\n");
	}
};

void destroy_ecs(ECS *ecs, PLT *plt)
{
	ECSEntry *cs = ecs->component_types;
	for (uint32 i = 0; i < NUM_COMPONENTS; i++)
	{
		if (cs[i]->c)
			plt->free(cs[i]->c);
	}
}

void run_system(SystemType type_id, ECS *ecs, float32 delta)
{
	ASSERT(type_id < NUM_SYSTEMS);
	switch (type_id)
	{
#define SYSTEM_ENTRY(id, func_name) case id: func_name(ecs, delta); break;

		SYSTEM_ENTRY(S_HELLO_WORLD, s_hello_world);

	default:
		ASSERT(!"Unknown systemtype!");
	}
}
