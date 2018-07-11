#pragma once

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

	bool operator== (EntityID o)
	{
		return pos == o.pos && uid == o.uid;
	}

	bool operator!= (EntityID o)
	{
		return !(*this == o);
	}
};

#define COMPONENT \
	union \
	{\
		BaseComponent base;\
		struct \
		{\
			int32 pos;\
			int32 uid;\
			ComponentType type;\
		};\
	};\

struct BaseComponent
{
	EntityID owner;
	ComponentType type;
};

struct Position
{
	COMPONENT
	Vec3f position;
};

struct Blargh
{
	COMPONENT
	int32 b;
	int32 a;
};

struct Entity
{
	EntityID id;
	int32 components[NUM_COMPONENTS];
};

struct ECSEntry
{
	int32 component_size; // Size of a component of this type.

	uint32 max_length;
	uint32 length;
	void *c; 
};

struct ECS
{
	ECSEntry component_types[NUM_COMPONENTS];

	int32 uid_counter = 1;
	int32 free_entity;
	int32 max_entity = -1;
	int32 allocated_entities;
	Entity *entities;
};

