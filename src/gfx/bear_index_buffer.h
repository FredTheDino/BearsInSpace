#pragma once

namespace GFX
{
	struct IndexBuffer
	{
		uint32 id = 0;
	};

	IndexBuffer create_index_buffer(uint64 size, uint32 usage=GL_STATIC_DRAW)
	{
		IndexBuffer buffer;
		glGenBuffers(1, &buffer.id);

		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, buffer.id);

		glBufferData(GL_ELEMENT_ARRAY_BUFFER, size * sizeof(uint32), NULL, usage);

		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);

		return buffer;
	}

	IndexBuffer create_index_buffer(Array<uint32> data, uint32 usage=GL_STATIC_DRAW)
	{
		IndexBuffer buffer;
		glGenBuffers(1, &buffer.id);
		
		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, buffer.id);

		glBufferData(GL_ELEMENT_ARRAY_BUFFER, size(data) * sizeof(uint32), data_ptr(data), usage);

		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);

		return buffer;
	}

	void bind(IndexBuffer buffer)
	{
		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, buffer.id);
	}

	void sub_data(IndexBuffer buffer, int64 offset, Array<uint32> data)
	{
		bind(buffer);
		glBufferSubData(GL_ELEMENT_ARRAY_BUFFER, offset * sizeof(uint32), size(data) * sizeof(uint32), data_ptr(data));
	}

	void delete_index_buffer(IndexBuffer buffer)
	{
		glDeleteBuffers(1, &buffer.id);
	}
}
