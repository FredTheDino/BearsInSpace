#pragma once

#include "bear_texture.h"

struct FrameBuffer
{
	uint32 id;
	Array<Texture> textures;
	uint32 width, height;
	uint32 depth_render_buffer_id;
};
