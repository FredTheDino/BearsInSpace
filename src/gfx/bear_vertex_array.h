#pragma once

#include "bear_vertex_buffer.h"
#include "bear_index_buffer.h"

namespace GFX
{
	struct VertexAttribute
	{
		VertexBuffer vertex_buffer;
		uint32       location;
		uint32       size;
		uint32       type;
		bool         normalized = false;
		int32        stride = 0;
		const void  *offset = NULL;
	};

	struct VertexArray
	{
		uint32 id = 0;
		bool has_indices = false;
	};

	VertexArray create_vertex_array(Array<VertexAttribute> attribs, IndexBuffer index_buffer={})
	{
		VertexArray vertex_array;
		glGenVertexArrays(1, &vertex_array.id);

		glBindVertexArray(vertex_array.id);

		if (index_buffer.id > 0)
		{
			vertex_array.has_indices = true;
			bind(index_buffer);
		}

		for (uint32 i = 0; i < size(attribs); i++)
		{
			VertexAttribute attr = get(attribs, i);
			bind(attr.vertex_buffer);
			glEnableVertexAttribArray(attr.location);
			glVertexAttribPointer(attr.location, attr.size, attr.type, attr.normalized ? GL_TRUE : GL_FALSE, attr.stride, attr.offset);
		}

		glBindVertexArray(0);

		return vertex_array;
	}

	void bind(VertexArray arr)
	{
		glBindVertexArray(arr.id);
	}

	void delete_vertex_array(VertexArray arr)
	{
		glDeleteVertexArrays(1, &arr.id);
	}
}
