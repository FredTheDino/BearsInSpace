#pragma once

namespace GFX
{
	struct Texture
	{
		uint32 id;
	};

	Texture create_texture(uint32 width, uint32 height, 
			uint32 color_depth, uint8 *data)
	{
		ASSERT(color_depth == 4);
		Texture t = {};

		glGenTextures(1, &t.id);

		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_2D, t.id);

		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, width, height, 0, GL_BGRA, GL_UNSIGNED_BYTE, data);

		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);

		glBindTexture(GL_TEXTURE_2D, 0);
		
		return t;
	}
	
	void bind(Texture texture, uint8 target=0)
	{
		glActiveTexture(GL_TEXTURE0 + target);
		glBindTexture(GL_TEXTURE_2D, texture.id);
	}

	void delete_texture(Texture texture)
	{
		glDeleteTextures(1, &texture.id);
	}
}
