#pragma once

namespace GFX
{
	typedef uint32 VertexBuffer;

	VertexBuffer create_vertex_buffer(Array<float32> data, uint32 usage=GL_STATIC_DRAW)
	{
		VertexBuffer buffer;
		glGenBuffers(1, &buffer);

		glBindBuffer(GL_ARRAY_BUFFER, buffer);

		glBufferData(GL_ARRAY_BUFFER, size(data) * sizeof(float32), data_ptr(data), usage);

		glBindBuffer(GL_ARRAY_BUFFER, 0);

		return buffer;
	}

	void bind(VertexBuffer buffer)
	{
		glBindBuffer(GL_ARRAY_BUFFER, buffer);
	}

	void delete_vertex_buffer(VertexBuffer buffer)
	{
		glDeleteBuffers(1, &buffer);
	}
}
