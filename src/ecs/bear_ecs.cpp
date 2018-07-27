#include "bear_ecs.h"

Entity* get_entity(ECS *ecs, EntityID id)
{
	if ((id._index > ecs->max_entity) ||
		(0 > id._index) ||
		(0 > id._uid)) return NULL;
	Entity *e = &ecs->entities[id._index];
	if (e->id == id)
		return e;
	return NULL;
}

inline
int8 *get_component(const ECSEntry entry, const int32 index)
{
	ASSERT(index < (const int32) entry.length);
	return ((int8*) entry.c) + index * entry.component_size;
}

BaseComponent *get_component(ECS *ecs, Entity *entity, ComponentType type)
{
	int32 component_id = entity->components[type];
	if (component_id < 0) return NULL;
	return (BaseComponent *) get_component(ecs->component_types[type], component_id);
}

BaseComponent *get_component(ECS *ecs, EntityID id, ComponentType type)
{
	Entity *entity = get_entity(ecs, id);
	if (!entity) return NULL;
	return (BaseComponent *) get_component(ecs, entity, type);
}

#define get_smallest_type(ecs, ...) get_fastest_type_list(ecs, sizeof((ComponentType []) {__VA_ARGS__}), {__VA_ARGS__})
int32 get_smallest_type_list(ECS *ecs, int32 num, ComponentType types[])
{
	ECSEntry *entries = ecs->component_types;
	ComponentType smallest_type = types[0];
	int32 smallest_length = entries[smallest_type].length;
	for (int32 i = 1; i < num; i++)
	{
		ComponentType type = types[i];
		int32 length = entries[type].length;
		if (smallest_length < length)	
		{
			smallest_length = length;
			smallest_type = type;
		}
	}
	return smallest_type;
}

#define entity_has_components(entity, ...) entity_has_components_list(entity, \
		sizeof((int32 []) {__VA_ARGS__}), {__VA_ARGS__})
bool entity_has_components_list(Entity entity, int32 num, ComponentType types[])
{
	for (int i = 0; i < num; i++)
	{
		if (entity.components[types[i]] < 0)
			return false;
	}
	return true;
}

#define id_has_components(ecs, id, ...) id_has_components_list(ecs, id, \
		sizeof((int32 []) {__VA_ARGS__}), {__VA_ARGS__})
bool id_has_components_list(ECS *ecs, EntityID id, int32 num, ComponentType types[])
{
	Entity *entity = get_entity(ecs, id);
	if (entity)
		return entity_has_components_list(*entity, num, types);
	return false;
}


static void write_component(const ECSEntry entry, const int32 to_index, const BaseComponent *comp)
{
	int32 size = entry.component_size;
	int8 *from = (int8 *) comp;
	int8 *to   = get_component(entry, to_index);
	while (size)
	{
		*to++ = *from++;
		size--;
	}
}

static void write_component(const ECSEntry entry, const int32 to_index, const int32 from_index)
{
	int32 size = entry.component_size;
	int8 *from = get_component(entry, from_index);
	int8 *to   = get_component(entry, to_index);
	while (size)
	{
		*to++ = *from++;
		size--;
	}
}

bool add_component(ECS *ecs, EntityID id, ComponentType type, BaseComponent *component)
{
	// NOTE(Ed): This is SUPER risqué. But you should ONLY pass in components. 
	// All of which have this as a first field. So it should be safe.
	ASSERT(id._index <= ecs->max_entity);
	ASSERT(0 <= id._index);
	ASSERT(0 <= id._uid);
	ASSERT(type < NUM_COMPONENTS);
	ASSERT(0 <= type);

	Entity *entity = get_entity(ecs, id);
	if (!entity) return false;

	component->type = type;
	component->owner = id;

	ECSEntry entry = ecs->component_types[type];
	if (entry.max_length == entry.length || entry.max_length == 0)
	{
		entry.max_length += 50;
		void *tmp = REALLOC(entry.c, entry.component_size * entry.max_length);
		if (!tmp) return false;
		entry.c = tmp;
	}

	int32 component_id = entry.length++;
	ASSERT(component_id >= 0);
	ASSERT(component_id < (int32) entry.length);
	write_component(entry, component_id, component);

	entity->components[type] = component_id;
	ecs->component_types[type] = entry;

	return true;
}

bool add_component(ECS *ecs, EntityID id, BaseComponent *component)
{
	return add_component(ecs, id, component->type, component);
}

bool add_body_component(ECS *ecs, Physics *phy, EntityID id, BaseComponent *component)
{
	ASSERT(component->type == C_BODY);
	bool success = add_component(ecs, id, C_BODY, component);
	if (success)
	{
		success = add_body(phy, id);
	}
	return success;
}

// NOTE(Ed): Only call this from the macro. The list __HAS TO BE NULL TERMINATED__!
#define add_components(ecs, id, ...) add_components_(ecs, id, __VA_ARGS__, NULL)
void add_components_(ECS *ecs, Physics *phy, EntityID id, ...)
{
	va_list args;
	va_start(args, id);
	while (true)
	{
		BaseComponent *component = va_arg(args, BaseComponent *);
		if (!component) break;
		if (component->type == C_BODY)
			if (phy)
				add_body_component(ecs, phy, id, component);
			else	
				DEBUG_LOG("Adding Body component without giving in physics engine\n");
		else
			add_component(ecs, id, component->type, component);
	}
	va_end(args);
}

bool remove_component(ECS *ecs, Entity *entity, ComponentType type)
{
	int32 component_offset = entity->components[type];
	if (component_offset < 0)
		return false;

	ECSEntry entry = ecs->component_types[type];
	BaseComponent *component_to_remove = (BaseComponent *) get_component(entry, component_offset);
	if (component_to_remove->owner != entity->id) return false;

	int32 largest_index = entry.length - 1;
	if (component_offset != largest_index)
	{
		write_component(entry, component_offset, largest_index);
		EntityID owner = ((BaseComponent *) get_component(entry, component_offset))->owner;
		get_entity(ecs, owner)->components[type] = component_offset;
	}
	entry.length--;
	entity->components[type] = -1;
	ecs->component_types[type] = entry;

	return true;

}

bool remove_component(ECS *ecs, EntityID id, ComponentType type)
{
	Entity* entity = get_entity(ecs, id);
	if (entity)
		return remove_component(ecs, entity, type);
	return false;
}

#define remove_components(ecs, id, ...) remove_components_(ecs, id, __VA_ARGS__, NUM_COMPONENTS)
void remove_components_(ECS *ecs, EntityID id, ...)
{
	Entity *entity = get_entity(ecs, id);
	if (!entity) return;

	va_list args;
	va_start(args, id);
	while (true)
	{
		ComponentType type = (ComponentType) va_arg(args, int32);
		if (type == NUM_COMPONENTS) break;
		remove_component(ecs, entity, type);
	}
	va_end(args);
}

EntityID add_entity(ECS *ecs)
{
	// TODO: This should be refactored into a new data structure. 
	// Since we have this in 3 places. (Add/Remove Entity, Buffer and Source.
	EntityID id;
	id._uid = ecs->uid_counter++;
	if (ecs->uid_counter < 0)
		ecs->uid_counter = 1;

	if (ecs->free_entity < 0)
	{
		id._index = -ecs->free_entity - 1;
		ecs->free_entity = ecs->entities[id._index].id._index;
	}
	else
	{
		id._index = ecs->free_entity++;
	}

	if (ecs->allocated_entities <= id._index)
	{
		int32 new_allocation_size = ecs->allocated_entities	* 2;
		Entity *ptr = (Entity *) REALLOC(ecs->entities, new_allocation_size);
		if (!ptr) return {-1, -1};
		ecs->entities = ptr;
		ecs->allocated_entities = new_allocation_size;
	}

	ecs->max_entity = maximum(ecs->max_entity, (int32) id._index);

	Entity entity = {id};

	for (int32 i = 0; i < NUM_COMPONENTS; i++)
	{
		entity.components[i] = -1;
	}

	ecs->entities[id._index] = entity;
	return id;
}

void remove_entity(ECS *ecs, EntityID id)
{
	// TODO: This should be refactored into a new data structure. 
	// Since we have this in 3 places. (Add/Remove Entity, Buffer and Source.
	Entity *entity = &ecs->entities[id._index];
	if (!(entity->id == id)) return;
	
	// Remove components
	for (int32 i = 0; i < NUM_COMPONENTS; i++)
	{
		remove_component(ecs, entity, (ComponentType) i);
	}
	
	int32 pos = id._index;
	id._index = ecs->free_entity;
	id._uid = -1;
	ecs->free_entity = -pos - 1;
	ecs->entities[pos].id = id;

	if (pos == ecs->max_entity)
	{
		while (ecs->entities[ecs->max_entity].id._uid < 0 && 0 <= ecs->max_entity)
			ecs->max_entity--;
	}
}

void clear_ecs(World *world)
{
	clear(&world->phy.body_limits);

	world->ecs.free_entity = 0;
	world->ecs.max_entity = -1;

	for (int32 i = 0; i < NUM_COMPONENTS; i++)
	{
		world->ecs.component_types[i].length = 0;
	}
}

void s_hello_world(World *world, float32 delta)
{
	world->plt.print("Delta is %.2f\n", delta);
}

void update_physics(ECS *ecs, Physics *, float32);

void run_system(SystemType type_id, World *world, float32 delta)
{
	ASSERT(type_id < NUM_SYSTEMS);
	switch (type_id)
	{
#define SYSTEM_ENTRY(id, func_call) case id: (func_call); break;

		SYSTEM_ENTRY(S_HELLO_WORLD, s_hello_world(world, delta));
		SYSTEM_ENTRY(S_PHYSICS, update_physics(&world->ecs, &world->phy, delta));

	default:
		ASSERT(!"Unknown systemtype!");
	}
}

