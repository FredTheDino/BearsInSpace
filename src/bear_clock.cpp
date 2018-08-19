#include "bear_clock.h"

const char *main_clock = "Main Clock";
void reset_debug_clock()
{
	DebugCLK clock = {(char *) main_clock, plt.get_time(), 0.0};
	world->clk.clocks[0] = clock;
	world->clk.num_clocks = 1;
}

CLKID start_debug_clock(const char *name)
{
	CLKID id = world->clk.clock_id++;
	float64 start_time = plt.get_time();
	DebugCLK clock = {(char *) name, start_time, 0.0};
	world->clk.clocks[id] = clock;
	return id;
}

void stop_debug_clock(CLKID id)
{
	float64 end_time = plt.get_time();
	world->clk.clocks[id].end_time = end_time;
}

void display_clocks()
{
	PRINT("====== CLOCKS ========\n");
	float64 whole = plt.get_time() - world->clk.clocks[0].start_time;
	PRINT("   Whole: %.10f\n", whole);
	for (uint64 i = 1; i < world->clk.num_clocks; i++)
	{
		DebugCLK clock = world->clk.clocks[i];
		float64 time_taken = clock.end_time - clock.start_time;
		float64 part_time = time_taken / whole;
		PRINT("    %s, %.10f (%.2f%%)\n", clock.name, time_taken, part_time);
	}
}
