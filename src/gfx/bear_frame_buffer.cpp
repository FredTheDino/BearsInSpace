#include "bear_frame_buffer.h"

namespace GFX
{
	FrameBuffer create_frame_buffer(Array<uint32> attachments, uint32 width, uint32 height)
	{
		ASSERT(size(attachments) > 0);
		
		FrameBuffer buffer = {};
		buffer.width = width;
		buffer.height = height;
		
		glGenFramebuffers(1, &buffer.id);

		glBindFramebuffer(GL_DRAW_FRAMEBUFFER, buffer.id);
		
		buffer.textures = static_array<Texture>(size(attachments));
		
		bool has_depth_texture = false;

		Array<uint32> draw_buffers = temp_array<uint32>(size(attachments));
		
		for (uint32 i = 0; i < size(attachments); i++)
		{
			Texture t;
			if (attachments[i] == GL_DEPTH_ATTACHMENT)
			{
				has_depth_texture = true;
				t = create_texture(width, height, true);
				set(draw_buffers, i, (uint32) GL_NONE);
			}
			else
			{
				t = create_texture(width, height);
				set(draw_buffers, i, (uint32) attachments[i]);
			}

			append(&buffer.textures, t);

			glFramebufferTexture2D(GL_DRAW_FRAMEBUFFER, attachments[i], GL_TEXTURE_2D, t.id, 0);
		}

		glDrawBuffers(size(attachments), draw_buffers.data);

		// We still need depth rendering for depth test
		if (!has_depth_texture)
		{
			glGenRenderbuffers(1, &buffer.depth_render_buffer_id);
			glBindRenderbuffer(GL_RENDERBUFFER, buffer.depth_render_buffer_id);
			glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT, width, height);
			glFramebufferRenderbuffer(GL_DRAW_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, buffer.depth_render_buffer_id);
		}
		
		return buffer;
	}

	void bind(FrameBuffer buffer)
	{
		glBindFramebuffer(GL_DRAW_FRAMEBUFFER, buffer.id);
		glViewport(0, 0, buffer.width, buffer.height);
	}

	void delete_frame_buffer(FrameBuffer buffer)
	{
		if (buffer.depth_render_buffer_id > 0)
			glDeleteRenderbuffers(1, &buffer.depth_render_buffer_id);
		for (uint32 i = 0; i < size(buffer.textures); i++)
			delete_texture(buffer.textures[i]);
		delete_array(&buffer.textures);
		glDeleteFramebuffers(1, &buffer.id);
	}
}
