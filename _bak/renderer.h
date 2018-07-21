#pragma once

#include "vertexarray.h"
#include "shaderprogram.h"
#include "transform.h"
#include "texture.h"

#include "bear_array.h"

namespace Renderer
{
	struct MatrixProfile
	{
		string               uniform_name;
		uint8                uniform_length;
		Transform           *transform;
	};
	
	struct Renderable
	{
		VertexArray         *vertex_array;
		ShaderProgram       *program;
		unsigned int         num_vertices;
		Texture             *texture;
		Array<MatrixProfile> matrix_profiles;
	};

	void draw(Renderable r, unsigned int mode=GL_TRIANGLES,
			  bool use_global_matrix_profiles=true)
	{
		r.program->bind();

		if (use_profiles)
			for (auto mp : _matrix_profiles)
				r.program->sendM4(mp.first.c_str(), mp.second->matrix());

		if (r.texture != nullptr)
			r.texture->bind();

		r.program->sendM4(r.uniform.c_str(), r.transform->matrix());

		r.vertex_array->bind();

		if (r.vertex_array->hasIndices())
			glDrawElements(mode, r.num_vertices, GL_UNSIGNED_INT, NULL);
		else
			glDrawArrays(mode, 0, r.num_vertices);
	}

	void add_profile(string uniform_name, uint8 uniform_length,
					 Transform *transform)
	{
		append(&matrix_profiles, { uniform_name, uniform_length, transform });
	}

	void remove_profile(string uniform_name, uint8 uniform_length)
	{
		uint64 size = size(matrix_profiles);
	}

	Array<MatrixProfile> matrix_profiles;
}
