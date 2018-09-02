#pragma once

namespace GFX
{
	void draw(FrameBuffer target, ECS *ecs, bool use_global_matrix_profiles=true)
	{
		bind(target);
		glClearColor(.1f, .1f, .1f, 1.0f);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
		
		debug_draw_engine(ecs, &world->phy);

#if 0
		draw_surface_text("open-sans", -.7f, .8f, "Testing!", .001f);
		draw_surface_text("open-sans", -.7f, .0f, "Testing!", .002f);
		draw_surface_text("open-sans", -.7f, -.8f, "Testing!", .004f);
#endif
	}
}
