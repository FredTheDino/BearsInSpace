#pragma once

namespace GFX
{
	typedef uint32 VertexBuffer;

	VertexBuffer create_vertex_buffer(float32 *data, uint32 size, uint32 usage=GL_STATIC_DRAW)
	{
		VertexBuffer buffer;
		glGenBuffers(1, &buffer);

		glBindBuffer(GL_ARRAY_BUFFER, buffer);

		glBufferData(GL_ARRAY_BUFFER, size * sizeof(float32), data, usage);

		glBindBuffer(GL_ARRAY_BUFFER, 0);

		return buffer;
	}

	// NOTE: I don't think we'll need this, like ever.
	VertexBuffer create_vertex_buffer(Array<float32> data, uint32 usage=GL_STATIC_DRAW)
	{
		return create_vertex_buffer(data_ptr(data), size(data), usage);
	}

	void bind(VertexBuffer buffer)
	{
		glBindBuffer(GL_ARRAY_BUFFER, buffer);
	}

	void sub_data(VertexBuffer buffer, int64 offset, Array<float32> data)
	{
		bind(buffer);
		glBufferSubData(GL_ARRAY_BUFFER, offset, size(data) * sizeof(float32), data_ptr(data));
	}

	void delete_vertex_buffer(VertexBuffer buffer)
	{
		glDeleteBuffers(1, &buffer);
	}
}
