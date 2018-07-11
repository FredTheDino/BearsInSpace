#pragma once

/* Input Type */
enum InputType
{
	KEY, MOUSE, JOYSTICK;
};

struct InputBinding
{
	InputType type;
	union
	{
		// Key
		SDLKey key;

		// Mouse
		uint8 m_button;

		// Joystick
		struct
		{
			uint8 j_device;
			uint8 j_button;
		};
	};
};

enum ButtonState
{
	UP, DOWN, PRESSED, RELEASED;
};
	
struct Button
{
	InputBinding binding;
	ButtonState state;
	Button *next_button;
};

struct Axis
{
	
};

struct InputEntry
{
	InputEntry   *next;
	string        name;
	Input         binding;
} input_map[255];

uint8 input_map_hash(string name)
{
	return (uint8) *str;
}

Input get_input(string name)
{
	InputEntry in = input_map[input_map_hash(name)];
	while (strcmp(in.name, name) != 0)
	{
		in = *in.next;
	}

	return in.binding;
}

void bind_key(uint16 id, int32 value)
{
	bind_input(SDL_KeyboardEvent, id, value);
}

void bind_mouse_button()
{
	bind_input(SDL_MouseButton);
}
