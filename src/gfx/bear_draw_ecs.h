#pragma once

void draw_game_world(FrameBuffer target, ECS *ecs)
{
	bind(target);
	glClearColor(.1f, .1f, .1f, 1.0f);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
}
