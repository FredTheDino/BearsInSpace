#include "bear_texture.h"

namespace GFX
{
	Texture create_texture(uint32 width, uint32 height, 
		uint32 internal_format=GL_RGBA, 
		uint32 format=GL_BGRA,
		uint32 filter=GL_LINEAR)
	{
		Texture t = {};
		t.width = width;
		t.height = height;

		glGenTextures(1, &t.id);

		glBindTexture(GL_TEXTURE_2D, t.id);

		glTexImage2D(GL_TEXTURE_2D, 0, internal_format, width, height, 0, format, GL_UNSIGNED_BYTE, (void *) 0);

		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, filter);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, filter);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);

		glBindTexture(GL_TEXTURE_2D, 0);

		return t;
	}

	Texture create_texture(uint32 width, uint32 height, 
			       uint32 color_depth, uint8 *data,
			       uint32 internal_format=GL_RGBA,
			       uint32 format=GL_BGRA,
			       uint32 filter=GL_LINEAR)
	{
		ASSERT(color_depth == 4);
		Texture t = {};
		t.width = width;
		t.height = height;

		glGenTextures(1, &t.id);

		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_2D, t.id);

		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, width, height, 0, GL_BGRA, GL_UNSIGNED_BYTE, data);

		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, filter);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, filter);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);

		glBindTexture(GL_TEXTURE_2D, 0);
		
		return t;
	}

#if 0 // Depricated
	Texture create_texture(Image img)
	{
		Texture t = {};
		t.width = img.width;
		t.height = img.height;
		
		glGenTextures(1, &t.id);

		glBindTexture(GL_TEXTURE_2D, t.id);

		glTexImage2D(GL_TEXTURE_2D, 0, internal_format, img.width, img.height, 0, format, GL_UNSIGNED_BYTE, img.data);

		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);

		glBindTexture(GL_TEXTURE_2D, 0);
		
		return t;
	}

	Texture create_texture(string path)
	{
		Image img = load_image(path);
		Texture t = create_texture(load_image(path));
		free_image(&img);
		return t;
	}
#endif

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
