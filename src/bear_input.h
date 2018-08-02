#pragma once

/* Input Type */
enum InputType
{
	KEY, MOUSE, CONTROLLER
};

#define INPUT_MAP_SIZE 256
#define MAX_AXIS    32
#define MAX_BUTTONS 32
#define CONTROLLER_AXIS_THRESHOLD .2
#define CONTROLLER_AXIS_FACTOR (1.0 / 32767.0)

#ifdef BEAR_GAME

#define AXIS_VAL(name) world->plt.axis_value(name)
#define B_STATE(name) world->plt.button_state(name)
#define B_PRESSED(name) (world->plt.button_state(name) == ButtonState::PRESSED)
#define B_RELEASED(name) (world->plt.button_state(name) == ButtonState::RELEASED)
#define B_DOWN(name) (world->plt.button_state(name) == ButtonState::DOWN)
#define B_UP(name) (world->plt.button_state(name) == ButtonState::UP)

#else

// Forwarding
struct Axis;

struct Button
{
	struct Binding
	{
		InputType type;
		union
		{
			// Key
			int32 key;

			// Mouse
			uint8 m_button;

			// Controller
			struct
			{
				SDL_JoystickID c_device;
				SDL_GameControllerButton c_button;
			};
		};

		bool operator== (Binding o)
		{
			if (type != o.type)
				return false;

			switch (type)
			{
			case InputType::KEY: return key == o.key;
			case InputType::MOUSE: return m_button == o.m_button;
			case InputType::CONTROLLER: return c_device == o.c_device && c_button == o.c_button;
			default: return false;
			}
		}
	} binding;
	ButtonState state;
	Array<Axis *> axises;
};

struct Axis
{
	struct Binding
	{
		InputType type;
		
		union
		{
			// Key
			struct
			{
				Button *k_positive;
				Button *k_negative;
			};
			
			// Mouse
			bool m_x;

			// Controller
			struct
			{
				SDL_JoystickID c_device;
				SDL_GameControllerAxis c_axis;
			};
		};

		bool operator== (Binding o)
		{
			if (type != o.type)
				return false;

			switch (type)
			{
			case InputType::KEY: return k_positive == o.k_positive && k_negative == o.k_negative;
			case InputType::MOUSE: return m_x == o.m_x;
			case InputType::CONTROLLER: return c_device == o.c_device && c_axis == o.c_axis;
			default: return false;
			}
		}
	} binding;
	AxisValue value;
};

struct AxisEntry
{
	AxisEntry    *next;
	string        name;
	Array<Axis *>  axises;
} axis_map[INPUT_MAP_SIZE];

Array<Axis> axis_array;

struct ButtonEntry
{
	ButtonEntry   *next;
	string         name;
	Array<Button *> buttons;
} button_map[INPUT_MAP_SIZE];

Array<Button> button_array;

//TODO: Replace with more efficient alternative
uint8 input_map_hash(string name)
{
	return strlen(name) > 1 ? // 0x61 = ascii value for 'a'
		(((((uint8) *name) - 0x61) & 0x0F) << 4) | ((((uint8) name[1]) - 0x61) & 0x0F)
		: (uint8) *name;
}

Array<Axis *> get_axises(string name)
{
	AxisEntry entry = axis_map[input_map_hash(name)];

	if (entry.name == nullptr)
	{
		ERROR_LOG("Invalid axis name:");
		ERROR_LOG(name);
		return {};
	}
	
	while (strcmp(entry.name, name))
	{
		if (entry.next == nullptr)
			return {};
		entry = *entry.next;
	}

	return entry.axises;
}

Array<Button *> get_buttons(string name)
{
	ButtonEntry entry = button_map[input_map_hash(name)];

	if (entry.name == nullptr)
	{
		ERROR_LOG("Invalid button name:");
		ERROR_LOG(name);
		return {};
	}
	
	while (strcmp(entry.name, name))
	{
		if (entry.next == nullptr)
			return {};
		entry = *entry.next;
	}
	
	return entry.buttons;
}

AxisValue axis_value(string name)
{
	Array<Axis *> axises = get_axises(name);
	AxisValue value = 0;
	for (uint8 i = 0; i < size(axises); i++)
	{
		Axis *a = get(axises, (uint64) i);
		value += a->value;
	}
	
	if (absolute(value) < CONTROLLER_AXIS_THRESHOLD)
		return 0;
	else
		return clamp(value, -1.0, 1.0);
}

ButtonState button_state(string name)
{
	Array<Button *> buttons = get_buttons(name);
	uint8 state_bits = 0;
	for (uint8 i = 0; i < size(buttons); i++)
	{
		Button *b = get(buttons, (uint64) i);
		state_bits |= (uint8) b->state;
	}
	
	if (state_bits & ButtonState::DOWN ||
		(state_bits & ButtonState::RELEASED
		 && state_bits & ButtonState::PRESSED))
		return ButtonState::DOWN;
	else if (state_bits & ButtonState::PRESSED)
		return ButtonState::PRESSED;
	else if (state_bits & ButtonState::RELEASED)
		return ButtonState::RELEASED;
	else
		return ButtonState::UP;
}

void add_to_map(string name, Button *button)
{
	ButtonEntry *entry = &button_map[input_map_hash(name)];
	
	if (size(entry->buttons) == 0)
	{
		entry->buttons = create_array<Button *>(1);
	}
	else
	{
		while (strcmp(entry->name, name))
		{
			if (entry->next == nullptr)
			{
				entry->next = new ButtonEntry;
				*entry->next = {};
				entry->next->buttons = {};
				entry->next->buttons = create_array<Button *>(1);
				entry = entry->next;
				break;
			}
			entry = entry->next;
		}
	}
	entry->name = name;
	append(&entry->buttons, button);
}

void add_to_map(string name, Axis *axis)
{
	AxisEntry *entry = &axis_map[input_map_hash(name)];

	if (size(entry->axises) == 0)
	{
		entry->axises = create_array<Axis *>(1);
	}
	else
	{
		while (strcmp(entry->name, name))
		{
			if (entry->next == nullptr)
			{
				entry->next = new AxisEntry;
				*entry->next = {};
				entry->next->axises = {};
				entry->next->axises = create_array<Axis *>(1);
				entry = entry->next;
				break;
			}
			entry = entry->next;
		}
	}
	entry->name = name;
	append(&entry->axises, axis);
}

Button *_write_button_to_array(Button button)
{
	bool new_button = true;
	// Is the button already recognized?
	uint8 i = 0;
	while (i < size(button_array))
	{
		Button b = get(button_array, (uint64) i);
		if (b.binding.type == button.binding.type && b.binding == button.binding)
		{
			new_button = false;
			break;
		}

		i++;
	}

	// If the button is not yet recognized, make sure it'll be
	if (new_button)
		append(&button_array, button);

	return get_ptr(button_array, i);
}

void bind_button_key(string name, int32 key)
{
	Button button = {};
	
	button.binding.type = InputType::KEY;
	button.binding.key = key;
	button.state = ButtonState::UP;
	
	add_to_map(name, _write_button_to_array(button));
}

void bind_button_mouse(string name, uint8 mouse_button)
{
	Button button = {};

	button.binding.type = InputType::MOUSE;
	button.binding.m_button = mouse_button;
	button.state = ButtonState::UP;
	
	add_to_map(name, _write_button_to_array(button));
}

void bind_button_controller(string name, SDL_JoystickID c_device, SDL_GameControllerButton c_button)
{
	Button button = {};

	button.binding.type = InputType::CONTROLLER;
	button.binding.c_device = c_device;
	button.binding.c_button = c_button;
	button.state = ButtonState::UP;

	add_to_map(name, _write_button_to_array(button));
}

void bind_axis_key(string name, int32 k_pos, int32 k_neg)
{
	ASSERT(k_pos != k_neg);
	
	Axis axis = {};

	Button button = {};
	button.binding.type = InputType::KEY;
	button.state = ButtonState::UP;

	button.binding.key = k_pos;
	Button *b_pos = _write_button_to_array(button);
	if (size(b_pos->axises) == 0)
		b_pos->axises = create_array<Axis*>(1);

	button.binding.key = k_neg;
	Button *b_neg = _write_button_to_array(button);
	if (size(b_neg->axises) == 0)
		b_neg->axises = create_array<Axis*>(1);

	axis.binding.type = InputType::KEY;
	axis.binding.k_positive = b_pos;
	axis.binding.k_negative = b_neg;
	
	append(&axis_array, axis);
	Axis *axis_ptr = get_ptr(axis_array, size(axis_array) - 1);
	append(&b_pos->axises, axis_ptr);
	append(&b_neg->axises, axis_ptr);
	
	add_to_map(name, axis_ptr);
}

Axis *_write_axis_to_array(Axis axis)
{
	bool new_axis = true;
	// Is the axis already recognized?
	uint8 i = 0;
	while (i < size(axis_array))
	{
		Axis a = get(axis_array, (uint64) i);
		if (a.binding.type == axis.binding.type && a.binding == axis.binding)
		{
			new_axis = false;
			break;
		}

		i++;
	}

	// If the axis is not yet recognized, make sure it'll be
	if (new_axis)
		append(&axis_array, axis);

	return get_ptr(axis_array, i);
}

void bind_axis_mouse(string name, bool m_x)
{
	Axis axis = {};

	axis.binding.type = InputType::MOUSE;
	axis.binding.m_x = m_x;

	add_to_map(name, _write_axis_to_array(axis));
}

void bind_axis_controller(string name, SDL_JoystickID c_device, SDL_GameControllerAxis c_axis)
{
	Axis axis = {};

	axis.binding.type = InputType::CONTROLLER;
	axis.binding.c_device = c_device;
	axis.binding.c_axis = c_axis;

	add_to_map(name, _write_axis_to_array(axis));
}

void handle_keyboard_event(SDL_Event event)
{
	if (event.key.repeat)
		return;
	
	for (uint8 i = 0; i < size(button_array); i++)
	{
		Button *b = get_ptr(button_array, i);
		if (b->binding.type == InputType::KEY && b->binding.key == event.key.keysym.sym)
		{
			b->state = event.key.state == SDL_RELEASED ? ButtonState::RELEASED : ButtonState::PRESSED;
			uint64 num_axises = size(b->axises);
			for (uint8 j = 0; j < num_axises; j++)
			{
				Axis *a = get(b->axises, j);
				if (a->binding.k_positive->binding.key == b->binding.key)
				{
					if (event.key.state == SDL_PRESSED)
						a->value += 1;
					else
						a->value -= 1;
				}
				else
				{
					if (event.key.state == SDL_PRESSED)
						a->value -= 1;
					else
						a->value += 1;
				}

				a->value = maximum(minimum(a->value, -1.0), 1.0);
			}
		}
	}
}

void handle_mouse_motion_event(SDL_Event event)
{
	for (uint8 i = 0; i < size(axis_array); i++)
	{
		Axis *a = get_ptr(axis_array, (uint64) i);
		if (a->binding.type == InputType::MOUSE)
		{
			if (a->binding.m_x)
				a->value = event.motion.xrel;
			else
				a->value = event.motion.yrel;
		}
	}
}

void handle_controller_button_event(SDL_Event event)
{
	for (uint8 i = 0; i < size(button_array); i++)
	{
		Button *b = get_ptr(button_array, (uint64) i);
		if (b->binding.type == InputType::CONTROLLER &&
			b->binding.c_device == event.cbutton.which &&
			b->binding.c_button == event.cbutton.button)
		{
			b->state = event.cbutton.state == SDL_PRESSED ? ButtonState::PRESSED : ButtonState::RELEASED;
		}
	}
}

void handle_controller_axis_event(SDL_Event event)
{
	for (uint8 i = 0; i < size(axis_array); i++)
	{
		Axis *a = get_ptr(axis_array, i);
		if (a->binding.type == InputType::CONTROLLER &&
			a->binding.c_device == event.caxis.which &&
			a->binding.c_axis == event.caxis.axis)
			a->value = clamp(event.caxis.value * CONTROLLER_AXIS_FACTOR, -1.0, 1.0);
	}
}

void handle_input_event(SDL_Event event)
{
	switch (event.type)
	{
	case SDL_KEYUP:
	case SDL_KEYDOWN:
		handle_keyboard_event(event);
		break;
	case SDL_MOUSEMOTION:
		handle_mouse_motion_event(event);
		break;
	case SDL_CONTROLLERBUTTONUP:
	case SDL_CONTROLLERBUTTONDOWN:
		handle_controller_button_event(event);
		break;
	case SDL_CONTROLLERAXISMOTION:
		handle_controller_axis_event(event);
	}
}

void update_input()
{
	for (uint8 i = 0; i < size(button_array); i++)
	{
		Button *b = get_ptr(button_array, i);
		switch (b->state)
		{
		case ButtonState::PRESSED: b->state = ButtonState::DOWN; break;
		case ButtonState::RELEASED: b->state = ButtonState::UP; break;
		default: break;
		}
	}

	for (uint8 i = 0; i < size(axis_array); i++)
	{
		Axis *a = get_ptr(axis_array, i);
		if (a->binding.type == InputType::MOUSE)
			a->value = 0;
	}
}

Array<SDL_GameController *> controller_array;

void close_controllers()
{

	int32 num_controllers = (int32) size(controller_array);
	for (int32 i = 0; i < num_controllers; i++)
	{
		SDL_GameController *c = get(controller_array, i);
		if (SDL_GameControllerGetAttached(c))
			SDL_GameControllerClose(c);
	}
	
	delete_array(&controller_array);
}

void open_controllers()
{
	int32 num_joysticks = SDL_NumJoysticks();

	if (num_joysticks <= 0)
		return;

	// Close existing joysticks
	if (size(controller_array) > 0)
		close_controllers();

	controller_array = create_array<SDL_GameController *>(num_joysticks);
	
	for (int32 i = 0; i < num_joysticks; i++)
	{
		if (!SDL_IsGameController(i))
			continue;
		
		SDL_GameController *c = SDL_GameControllerOpen(i);

		if (!c)
		{
			ERROR_LOG("Failed to open controller!");
			continue;
		}
		
		DEBUG_LOG("Found controller:");
		DEBUG_LOG(SDL_GameControllerName(c));

		append(&controller_array, c);
	}
}

void init_input()
{
	button_array = create_array<Button>(MAX_BUTTONS);
	axis_array = create_array<Axis>(MAX_AXIS);
	for (int16 i = 0; i < INPUT_MAP_SIZE; i++)
	{
		button_map[i] = {};
		button_map[i].buttons = {};
		axis_map[i] = {};
		axis_map[i].axises = {};
	}

	controller_array = {};
	SDL_GameControllerEventState(SDL_ENABLE);
	open_controllers();
	
	bind_axis_controller("xmove", 0, SDL_CONTROLLER_AXIS_LEFTX);
	bind_axis_controller("zmove", 0, SDL_CONTROLLER_AXIS_LEFTY);
	
	bind_axis_controller("xrot", 0, SDL_CONTROLLER_AXIS_RIGHTX);
	bind_axis_controller("yrot", 0, SDL_CONTROLLER_AXIS_RIGHTY);
	
	bind_axis_controller("up", 0, SDL_CONTROLLER_AXIS_TRIGGERRIGHT);
	bind_axis_controller("down", 0, SDL_CONTROLLER_AXIS_TRIGGERLEFT);

	bind_button_controller("forward", 0, SDL_CONTROLLER_BUTTON_A);
	bind_button_controller("left", 0, SDL_CONTROLLER_BUTTON_B);
	bind_button_controller("right", 0, SDL_CONTROLLER_BUTTON_X);
}

void destroy_input()
{
	for (uint16 i = 0; i < size(button_array); i++)
	{
		Button *b = get_ptr(button_array, (uint64) i);
		if (size(b->axises) > 0)
				delete_array(&b->axises);
	}

	for (uint16 i = 0; i < INPUT_MAP_SIZE; i++)
	{
		ButtonEntry *b = &button_map[i];
		ButtonEntry *next_b = b->next;
		if (size(b->buttons) > 0)
			delete_array(&b->buttons);
		while (next_b != nullptr)
		{
			b = next_b;
			if (size(b->buttons) > 0)
				delete_array(&b->buttons);
			
			next_b = next_b->next;
			delete b;
		}

		AxisEntry *a = &axis_map[i];
		AxisEntry *next_a = a->next;
		if (size(a->axises) > 0)
			delete_array(&a->axises);
		while (next_a != nullptr)
		{
			a = next_a;
			if (size(a->axises) > 0)
				delete_array(&a->axises);
			
			next_a = next_a->next;
			delete a;
		}
	}

	delete_array(&button_array);
	delete_array(&axis_array);

	close_controllers();
}

#endif
