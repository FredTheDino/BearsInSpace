#pragma once

struct DebugCLK
{
	char *name;
	float64 start_time;
	float64 end_time;
};

typedef uint64 CLKID;

struct CLK
{
	float32 delta;

	// TODO: Remove in none debug.
	union
	{
		CLKID clock_id;
		uint64 num_clocks;
	};
	DebugCLK clocks[1024];
};

