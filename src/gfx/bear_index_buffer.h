#pragma once

namespace GFX
{
	struct IndexBuffer
	{
		uint32 id = 0;
	};

	IndexBuffer create_index_buffer(uint32 *data, uint64 size, uint32 usage=GL_STATIC_DRAW)
	{
		IndexBuffer buffer;
		glGenBuffers(1, &buffer.id);
		
		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, buffer.id);

		glBufferData(GL_ELEMENT_ARRAY_BUFFER, size * sizeof(uint32), data, usage);

		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);

		return buffer;
	}
	
	IndexBuffer create_index_buffer(Array<uint32> data, uint32 usage=GL_STATIC_DRAW)
	{
		return create_index_buffer(data_ptr(data), size(data), usage);
	}

	void bind(IndexBuffer buffer)
	{
		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, buffer.id);
	}

	void delete_index_buffer(IndexBuffer buffer)
	{
		glDeleteBuffers(1, &buffer.id);
	}
}
