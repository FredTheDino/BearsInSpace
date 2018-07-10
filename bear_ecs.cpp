#include "bear_ecs.h"

Entity* get_entity(ECS *ecs, EntityID id)
{
	if ((id.pos < ecs->num_entities) ||
		(0 <= id.pos) ||
		(0 <= id.uid)) return NULL;
	Entity *e = ecs->entities[id.pos];
	if (*e.id == id)
		return e;
	return NULL;
}

inline static
int8 *get(const ECSEntry entry, const int32 index)
{
	return ((int8*) entry.c) + index * entry.size;
}

static void write_component(const ECSEntry entry, const int32 to_index, const BaseComponent *comp)
{
	int32 size = entry.size;
	int8 *from = (uint8 *) comp;
	int8 *to   = get(entry, to_index);
	while (size)
	{
		*to++ = *from++;
		size--;
	}
}

static void write_component(const ECSEntry entry, const int32 to_index, const int32 from_index)
{
	int32 size = entry.size;
	int8 *from = get(entry, from_index);
	int8 *to   = get(entry, to_index);
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
	ASSERT(id.pos < ecs->num_entities);
	ASSERT(0 <= id.pos);
	ASSERT(0 <= id.uid);
	ASSERT(component.type < NUM_COMPONENTS);
	ASSERT(0 < component.type);

	Entity *entity = get_entity(ecs, id);
	if (!entity) return false;

	component->owner = id;

	ECSEntry entry = ecs->component_types[type];
	if (entry.max == entry.num || entry.max == 0)
	{
		entry.max += 50;
		C *tmp = REALLOC(components, C, entry.max);
		if (!tmp) return false;
		entry.c = (void *) tmp;
	}

	int32 component_id = entry.num++;
	ASSERT(component_id >= 0);
	ASSERT(component_id < entry.num);
	write_component(entry, component_id, component);

	entity->components[type] = component_id;
	ecs->component_types[type] = entry;
	return true;
}

// TODO(Ed): Make a varadic arguments version.

#if 0
template<typename C>
bool add_component(ECS *ecs, EntityID id, ComponentType type, C c)
{
	c.type = type;
	return add_component<C>(ecs, id, c);
}
#endif

bool remove_component(ECS *ecs, int32 component_offset, ComponentType type)
{
	if (component_offset < 0)
		return false;

	ECSEntry entry = &ecs->component_types[type];
	if (component_offset != entry->num)
	{
		write_component(*entry, component_offset, entry->max;
		EntityID owner = ((BaseComponent *) get(*entry, component_offset))->owner;
		get_entity(owner)->components[type] = component_offset;
	}
	entry.num--;
	return true;
}

void remove_component(ECS *ecs, EntityID id, ComponentType type)
{
	Entity* entity = get_entity(ecs, id);
	if (entity)
		remove_component(ecs, entity->components[type], type);
}

EntityID add_entity(ECS *ecs)
{
	// TODO: This should be refactored into a new data structure. 
	// Since we have this in 3 places. (Add/Remove Entity, Buffer and Source.
	EntityID id;
	id.uid == ecs->uid_counter++;
	ecs->uid_counter &= 0x7FFFFF;

	if (ecs->free_entity < 0)
	{
		id.pos = -ecs->free_entity - 1;
		ecs->free_entity = ecs->entities[id.pos].id.pos;
	}
	else
	{
		id.pos = ecs->free_entity++;
	}

	if (ecs->allocated_entities <= id.pos)
	{
		int32 new_allocation_size = ecs->allocated_entities	* 2;
		Entity *ptr = REALLOC(ecs->entities, new_allocation_size);
		if (!ptr) return {-1, -1};
		ecs->entities = ptr;
		ecs->allocated_entities = new_allocation_size;
	}

	ecs->max_entity = maximum(ecs->max_entity, (int32) id.pos);
	Entity entity = {id};
	ecs->entities[id.pos] = entity;
}

void remove_entity(ECS *ecs, EntityID id)
{
	// TODO: This should be refactored into a new data structure. 
	// Since we have this in 3 places. (Add/Remove Entity, Buffer and Source.
	Entity entity = ecs->entities[id.pos];
	if (entity.id != id) return;
	
	// Remove components
	for (int32 i = 0; i < NUM_COMPONENTS; i++)
	{
		remove_component(ecs, entity.components[type], i);
	}
	
	uint32 pos = id.pos;
	id.pos = ecs->free_source;
	id.uid = -1;
	ecs->free_entity = -pos - 1;
	ecs->entity[pos].id = id;

	if (pos == ecs->max_entity)
	{
		while (ecs->entities[ecs->max_entity].id.uid < 0 && 0 <= ecs->max_ecs)
			ecs->max_ecs--;
	}

}
